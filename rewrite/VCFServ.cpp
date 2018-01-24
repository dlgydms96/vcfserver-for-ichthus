//------------------------------------------------------------------------------
// Taken from https://www.codeproject.com/Articles/1169105/Cplusplus-std-thread-Event-Loop-with-Message-Queue
//------------------------------------------------------------------------------


#include "VThread.h"
#include "Fault.h"
#include <iostream>

#include <fstream>
#include <google/protobuf/util/time_util.h>
#include "umsg.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;
using namespace vcf;
using namespace google::protobuf::io;

#define SERVER_IPADDR "127.0.0.1"
#define SERVER_PORTNO 9000
#define MAX_BUFFER 1024

///////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////

pthread_t recv_thread_id;
int glob_sockfd = -1;
int recv_count = 0;
int send_count = 0;
int ecat_count = 0;
int obd2_count = 0;

void send_handler(VThread *t, ThreadMsg *msg);
void ecat_handler(VThread *t, ThreadMsg *msg);
void obd2_handler(VThread *t, ThreadMsg *msg);
void exit_handler(VThread *t);

VThread *send_thread = NULL;
VThread *ecat_thread = NULL;
VThread *obd2_thread = NULL;

///////////////////////////////////////////////
// Socket and signal related functions
///////////////////////////////////////////////

void countdown(int sec) {
  for (int i = 0; i < sec; i++) {
    sleep(1);
    cout << "countdown to shutdown: " << sec-i << endl;
  }
}

void shutdown() { // shutdown server gracefully
  int retval = 0;
  if (recv_thread_id != 0) {
    pthread_cancel(recv_thread_id);
    pthread_join(recv_thread_id, (void **) &retval);
    if (retval == PTHREAD_CANCELED)
      cout << "recv_thread canceled" << endl;
    else
      cout << "recv_thread cancellation failed" << endl;
    recv_thread_id = 0;
  }
  ecat_thread->ExitThread();
  obd2_thread->ExitThread();
  send_thread->ExitThread();
  delete ecat_thread; ecat_thread = NULL;
  delete obd2_thread; obd2_thread = NULL;
  delete send_thread; send_thread = NULL;
  if (glob_sockfd >= 0) {
    close(glob_sockfd);
    glob_sockfd = -1;
  }
  countdown(3);
}

void signal_handler(int signo) {
  if (signo != SIGINT && signo != SIGTERM) {
    cout << "unexpected signal = " << signo << endl;
    exit(1);
  }

  shutdown();
  //exit(0);
}

int startup_server(char *ipaddr, int portno)
{
  struct sockaddr_in sockaddr;
  int sockfd;
  int val = 1;

  // create an unnamed socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // set a socket option to reuse the server address
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
    printf("error: setsockopt(): %s\n", strerror(errno));
    return -1;
  }

  // name the socket with the server address
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(ipaddr);
  sockaddr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
    printf("error: bind(): %s\n", strerror(errno));
    return -1;
  }

  // set the maximum number of pending connection requests
  if (listen(sockfd, 10) != 0) {
    printf("error: listen(): %s\n", strerror(errno));
    return -1;
  }

  return sockfd;
}

///////////////////////////////////////////////
// Handlers for send, ecat, obd2 threads
///////////////////////////////////////////////

void exit_handler(VThread *t) { 
  cout << t->get_thread_name() << " terminated" << endl;
}

void send_handler(VThread *t, ThreadMsg *msg) {
  const Umsg* umsg = static_cast<const Umsg*>(msg->msg);
  cout<<"size after serilizing is "<<umsg->ByteSize()<<endl;
  int siz = umsg->ByteSize()+1;
  char *pkt = new char [siz];
  google::protobuf::io::ArrayOutputStream aos(pkt,siz);
  CodedOutputStream *coded_output = new CodedOutputStream(&aos);
  coded_output->WriteVarint32(umsg->ByteSize());
  umsg->SerializeToCodedStream(coded_output);

  cout<<"in send_handler -> BODY: bytecount = " << siz << endl;
  for (int i = 0; i < siz; i++) {
      printf("_%d", pkt[i]);
  }
  printf("\n");

  if (write(glob_sockfd, pkt, siz) < 0) {
    cout << t->get_thread_name() << ": " << strerror(errno) << endl;
    kill(getpid(), SIGINT);
  }
  cout<<"after write in send_handler"<<endl;
}

void ecat_handler(VThread *t, ThreadMsg *msg) {
}

void obd2_handler(VThread *t, ThreadMsg *msg) {
}


///////////////////////////////////////////////
// Main routine for recv thread
///////////////////////////////////////////////
#include <regex.h>

// regex with one arg
regex_t regex_help;
regex_t regex_quit;
regex_t regex_show;  

// regex with two args
regex_t regex_get;

// regex with three args
regex_t regex_set;
regex_t regex_save;
regex_t regex_load;

// ???
regex_t regex_vcf_pos_reset;
regex_t regex_vcf_reset_pid;
regex_t regex_vcf_init;
regex_t regex_vcf_on;
regex_t regex_vcf_off;
regex_t regex_vcf_end;
regex_t regex_vcf_reset;
regex_t regex_obd_init;
regex_t regex_obd_on;
regex_t regex_obd_off;
regex_t regex_log_change;
regex_t set_pid_v;
regex_t regex_load_up;
regex_t regex_load_down;

void regcomp_all() { // NOT COMPLETE!!!
  regcomp(&regex_vcf_pos_reset, "^r$", REG_EXTENDED);
  regcomp(&regex_help, "^help$", REG_EXTENDED);
  regcomp(&regex_quit, "^quit$", REG_EXTENDED);
  regcomp(&regex_show, "^show$", REG_EXTENDED);
  regcomp(&regex_get, "^get ([a-zA-Z0-9_]{1,19})$", REG_EXTENDED);
  regcomp(&regex_set, "^set ([a-zA-Z0-9_]{1,19}) ([0-9]{1,19})$", REG_EXTENDED);
  regcomp(&regex_save, "^save ([a-zA-Z0-9_]{1,15}).([a-zA-Z0-9_]{1,3})$", REG_EXTENDED);
  regcomp(&regex_load, "^load ([a-zA-Z0-9_]{1,15}).([a-zA-Z0-9_]{1,3})$", REG_EXTENDED);
  regcomp(&regex_vcf_reset_pid, "^reset pid$", REG_EXTENDED);
  regcomp(&regex_vcf_init, "^vcf init$", REG_EXTENDED);
  regcomp(&regex_vcf_on, "^vcf on$", REG_EXTENDED);
  regcomp(&regex_vcf_off, "^vcf off$", REG_EXTENDED);
  regcomp(&regex_vcf_end, "^vcf end$", REG_EXTENDED);
  regcomp(&regex_vcf_reset, "^vcf reset$", REG_EXTENDED);
  regcomp(&regex_obd_init, "^obd init$", REG_EXTENDED);
  regcomp(&regex_obd_on, "^obd on$", REG_EXTENDED);
  regcomp(&regex_obd_off, "^obd off$", REG_EXTENDED);
  regcomp(&regex_log_change, "^log ([a-zA-Z0-9_]{1,15}).([a-zA-Z0-9_]{1,3})$", REG_EXTENDED);
  regcomp(&regex_load_up, "^up$", REG_EXTENDED);
  regcomp(&regex_load_down, "^down$", REG_EXTENDED);
}

void regfree_all() { // NOT COMPLETE!!!
  regfree(&regex_help);
  regfree(&regex_quit);
  regfree(&regex_show);
  regfree(&regex_get);
  regfree(&regex_set);
  regfree(&regex_save);
  regfree(&regex_load);  
}

int regexec_with_args(regex_t *regex, char *msg, int ngroups,
		      regmatch_t *groups, char *arg1, char *arg2) {
  int ret = regexec(regex, msg, ngroups, groups, 0);
  if (ret == 0)	{
    int len;
    if (ngroups > 1) {
      len = groups[1].rm_eo - groups[1].rm_so;
      memcpy(arg1, msg + groups[1].rm_so, len);
      arg1[len] = '\0';
    }
    if (ngroups > 2) {
      len = groups[2].rm_eo - groups[2].rm_so;
      memcpy(arg2, msg + groups[2].rm_so, len);
      arg2[len] = '\0';
    }
  }
  return !ret;
}

void chomp(char *msg, int len) {
  int nconsumed, nread = len;
  char *npos, *rpos, *cpos = msg;
  while ((npos = (char *) memchr(cpos, '\n', nread)) != NULL) {
    *npos = 0; // replace '\n' with NULL
    if ((rpos = (char *) memchr(cpos, '\r', nread)) != NULL)
      *rpos = 0; // replace '\r' with NULL
    nconsumed = npos - cpos + 1;
    nread -= nconsumed;
    cpos = npos + 1;
  }
}

void _recv_process(char *msg) { // NOT COMPLETE!!!
  regmatch_t groups[3];
  char arg1[128];
  char arg2[128];
  
  chomp(msg, strlen(msg)); // remove \r\n from message
  //regcomp_all();
  
  if (regexec_with_args(&regex_vcf_pos_reset, msg, 0, NULL, NULL, NULL)) {
    //do_vcf_pos_reset();
    cout << ": vcf_pos_reset" << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_help, msg, 0, NULL, NULL, NULL)) {
    //do_help();
    cout << ": help" << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_quit, msg, 0, NULL, NULL, NULL)) {
    //do_quit();
    cout << ": quit" << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_show, msg, 0, NULL, NULL, NULL)) {
    //do_show();
    cout << ": show " << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_get, msg, 2, groups, arg1, NULL)) {
    //do_get(arg1);
    cout << ": get " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_vcf_init, msg, 2, groups, arg1, NULL)) {
    //do_vcf_init(arg1);
    cout << ": vcf_init " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_vcf_on, msg, 2, groups, arg1, NULL)) {
    //do_vcf_on(arg1);
    cout << ": vcf_on " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_vcf_off, msg, 2, groups, arg1, NULL)) {
    //do_vcf_off(arg1);
    cout << ": vcf_off " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_vcf_end, msg, 2, groups, arg1, NULL)) {
    //do_vcf_end(arg1);
    cout << ": vcf_end " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_vcf_reset, msg, 2, groups, arg1, NULL)) {
    //do_vcf_reset(arg1);
    cout << ": vcf_reset " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_obd_init, msg, 2, groups, arg1, NULL)) {
    //do_obd_init(arg1);
    cout << ": obd_init " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_obd_on, msg, 2, groups, arg1, NULL)) {
    //do_obd_on(arg1);
    cout << ": obd_on " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_obd_off, msg, 2, groups, arg1, NULL)) {
    //do_obd_off(arg1);
    cout << ": obd_off " << arg1 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_log_change, msg, 3, groups, arg1, arg2)) {
    //do_log_change(arg1, arg2);
    cout << ": log_change " << arg1 << arg2 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_set, msg, 3, groups, arg1, arg2)) {
    //do_set(arg1, arg2);
    cout << ": set " << arg1 << arg2 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_save, msg, 3, groups, arg1, arg2)) {
    //do_save(arg1, arg2);
    cout << ": save " << arg1 << arg2 << endl;
    goto done;
  }
  else if (regexec_with_args(&regex_load, msg, 3, groups, arg1, arg2)) {
    //do_load(arg1, arg2);
    cout << ": load " << arg1 << arg2 << endl;
    goto done;
  }
  printf("Unknown command\n");

 done:
  //regfree_all();
  return;
}

int recv_process(char *msg) {
  Umsg* umsg = new Umsg();

  _recv_process(msg);
  send_thread->PostMsg(umsg);
  return 0;
}

void print_string(char *p) {
  while (*p != 0) printf("%u_", (unsigned char) *p++);
  printf("\n");
}

google::protobuf::uint32 readHdr(char *buf)
{
    google::protobuf::uint32 size;
    google::protobuf::io::ArrayInputStream ais(buf,4);
    CodedInputStream coded_input(&ais);
    coded_input.ReadVarint32(&size);
    cout<<"HDR: bytecount = " << 4 << endl;
    cout<<"HDR: content (in int32) = " << size << endl;
    return size;
}

Umsg *readBody(int sockfd,google::protobuf::uint32 siz)
{
     int bytecount;
     Umsg *umsg = new Umsg();
     char buffer [siz+4];//size of the payload and hdr
     //Read the entire buffer including the hdr
     printf("before recv function\n"); 
     if((bytecount = recv(sockfd, (void *)buffer, 1+siz, MSG_WAITALL))== -1)
     {
        fprintf(stderr, "Error receiving data %d\n", errno);
     }
     cout<<"BODY: bytecount = " << bytecount << endl;
     for (int i = 0; i < bytecount; i++) {
         printf("_%d", buffer[i]);
     }
     printf("\n");
     //Assign ArrayInputStream with enough memory 
     google::protobuf::io::ArrayInputStream ais(buffer,siz+4);
     CodedInputStream coded_input(&ais);
     //Read an unsigned integer with Varint encoding, truncating to 32 bits.
     coded_input.ReadVarint32(&siz);
     //After the message's length is read, PushLimit() is used to prevent the CodedInputStream 
     //from reading beyond that length.Limits are used when parsing length-delimited 
     //embedded messages
     google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);
     //De-Serialize
     umsg->ParseFromCodedStream(&coded_input);
     //Once the embedded message has been parsed, PopLimit() is called to undo the limit
     coded_input.PopLimit(msgLimit);
     //Print the message
     //cout<<"Message is "<<umsg->DebugString()<<endl;
     cout<<"_seqno : "<<umsg->_seqno()<<endl;
     cout<<"_from : "<<umsg->_from()<<endl;
     cout<<"_to : "<<umsg->_to()<<endl;
     cout<<"_cmd : "<<umsg->_cmd()<<endl;
     cout<<"_varID : "<<umsg->_varid()<<endl;
     cout<<"_value : "<<umsg->_value()<<endl;
     cout<<"_result : "<<umsg->_result()<<endl;
     cout<<"_timestamp : "<<umsg->_timestamp()<<endl;
     cout<<"_msg : "<<umsg->_msg()<<endl;
     return umsg;
}
void *recv_protoc(void *arg) 
{
    int *sockfd=(int *)arg;
    char buffer[4];
    int bytecount=0;
    string output,pl;
    Umsg *umsg;

    memset(buffer, '\0', 4);
    while(1)
    {
        if((bytecount= recv(*sockfd, buffer,4,MSG_PEEK))==-1)
        {
            fprintf(stderr, "Error receiving data %d\n",errno);
        }
        else if(bytecount==0) break;
        cout<<"peek bytes = "<< bytecount << endl;
        umsg = readBody(*sockfd, readHdr(buffer));
        send_thread->PostMsg((const Umsg *) umsg);
    }
}

void *recv_main(void *arg) {
  char buf[MAX_BUFFER], buffer[MAX_BUFFER];
  int sockfd = *((int *)arg);
  int nread, nconsumed, totread = 0;
  char *cpos, *npos, *rpos;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  memset(buffer, 0, sizeof(buffer));

  while (1) {
    memset(buf, 0, MAX_BUFFER);
    nread = read(sockfd, buf, sizeof(buf));
    if (nread < 0) {
      printf("recv_thread: error: read(): %s\n", strerror(errno));
      kill(getpid(), SIGINT);
    }
    else if (nread == 0) {
      printf("recv_thread: socket closed\n");
      kill(getpid(), SIGINT);
    }
    // else if (nread > 0)

    if (sizeof(buffer) - strlen(buffer) <= strlen(buf)) {
      printf("recv_thread: too small buffer\n");
      kill(getpid(), SIGINT);
    }

    //strcpy(buffer, "a\nbb\nccc"); totread = 8;
    strcat(buffer, buf);
    totread += nread;
    cpos = buffer;
    while ((npos = (char *) memchr(cpos, '\n', totread)) != NULL) {
      *npos = 0; // replace '\n' with NULL
      if ((rpos = (char *) memchr(cpos, '\r', totread)) != NULL)
	*rpos = 0; // replace '\r' with NULL
      printf("recv_thread: %s\n", cpos);
      if (recv_process(cpos) != 0)
	kill(getpid(), SIGINT);
      nconsumed = npos - cpos + 1;
      totread -= nconsumed;
      cpos = npos + 1;
    }
    //memset(buf, 0, MAX_BUFFER);
    strcpy(buf, cpos);
    memset(buffer, 0, MAX_BUFFER);
    strcpy(buffer, buf);
    if (strlen(buffer) > 0) {
      printf("recv_thread: %s (in buffer)\n", buffer);
      print_string(buffer);
    }
  }
}

///////////////////////////////////////////////
// Main function
///////////////////////////////////////////////

int main(int argc, char *argv[]) {
  sigset_t newmask, oldmask, waitmask;
  int c, flag_help = 0;
  char ipaddr[20] = SERVER_IPADDR;
  int portno = SERVER_PORTNO;
  int sockfd; // socket descriptors
  struct sockaddr_in sockaddr;
  
  while((c = getopt(argc, argv, "hi:p:")) != -1) {
    switch(c) {
    case 'h': flag_help = 1; break;
    case 'i': memcpy(ipaddr, optarg, strlen(optarg)); break;
    case 'c': portno = atoi(optarg); break;
    default:  printf("unknown option : %c\n", optopt); break;
    }
  }
  if (flag_help == 1) {
    printf("usage: %s [-h] [-i ipaddr] [-p portno] \n", argv[0]);
    exit(1);
  }
  printf("server address = %s:%d\n", ipaddr, portno);

  if (signal(SIGINT,  signal_handler) == SIG_ERR) {
    printf("error: signal(): %s\n", strerror(errno));
    exit(1);
  }
  if (signal(SIGTERM, signal_handler) == SIG_ERR) {
    printf("error: signal(): %s\n", strerror(errno));
    exit(1);
  }
  sigemptyset(&waitmask);
  sigaddset(&waitmask, SIGUSR1);
  sigemptyset(&newmask);
  sigaddset(&newmask, SIGINT);
  sigaddset(&newmask, SIGTERM);

  if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
    printf("error: sigprocmask(): %s\n", strerror(errno));
    exit(1);
  }
  
  if ((sockfd = startup_server(ipaddr, portno)) < 0) {
    shutdown();
    exit(1);
  }

  regcomp_all(); // compile all regex only once
  
  while (1) {
    int len = sizeof(sockaddr);
    glob_sockfd = accept(sockfd, (struct sockaddr *)&sockaddr, (socklen_t *)&len);
    if (glob_sockfd < 0) {
      printf("error: accept(): %s\n", strerror(errno));
      shutdown();
      break;
    }

    if (pthread_create(&recv_thread_id, NULL, &recv_protoc, (void *)&glob_sockfd) < 0) {
      printf("error: pthread_create(): %s\n", strerror(errno));
      shutdown();
      break;
    }

    send_thread = new VThread("send_thread", send_handler, exit_handler);
    ecat_thread = new VThread("ecat_thread", ecat_handler, exit_handler);
    obd2_thread = new VThread("obd2_thread", obd2_handler, exit_handler);
    ecat_thread->CreateThread();
    obd2_thread->CreateThread();
    send_thread->CreateThread();

    do { sigsuspend(&waitmask); } while(glob_sockfd >= 0);
  }

  regfree_all(); // free all regex

  if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
    printf("error: sigprocmask(): %s\n", strerror(errno));
    exit(1);
  }
  return 0;
}

///////////////////////////////////////////////
// end of file
///////////////////////////////////////////////
