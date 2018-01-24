#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <regex.h>

#define SERVER_IPADDR "127.0.0.1"
#define SERVER_PORTNO 9000
#define MAX_CLIENTS 2
#define MAX_BUFFER 1024
#define MAX_NAME 20

///////////////////////////////////////////////
// data structure (mutex not implemented!)
///////////////////////////////////////////////
struct client_struct {
  int id; 
  int sockfd;
  sockaddr_in sockaddr;
  char name[MAX_NAME];
} clients[MAX_CLIENTS];

int nclients = 0;
int client_id = 0;

void init_clients(void) {
  for (int k = 0; k < MAX_CLIENTS; k++) {
    clients[k].sockfd = -1;
  }
}

int find_empty_client(void) {
  for (int k = 0; k < MAX_CLIENTS; k++) {
    if (clients[k].sockfd < 0) return k;
  }
  return -1;
}

int find_client_by_id(int id) {
  for (int k = 0; k < MAX_CLIENTS; k++) {
    if (clients[k].sockfd < 0) continue;
    if (clients[k].id == id) return k;
  }
  return -1;
}

int find_client_by_name(char *name) {
  for (int k = 0; k < MAX_CLIENTS; k++) {
    if (clients[k].sockfd < 0) continue;
    if (!strcmp(clients[k].name, name)) return k;
  }
  return -1;
}

void add_client(int id, int sockfd, sockaddr_in sockaddr) {
  int idx = find_empty_client();
  if (idx < 0) {
    printf("error: no empty entries\n");
    exit(1);
  }
    
  clients[idx].id = id;
  clients[idx].sockfd = sockfd;
  clients[idx].sockaddr = sockaddr;
  sprintf(clients[idx].name, "anonymous_%d", sockfd);
  printf("client[%d] added (id=%d,name=%s)\n", idx, id, clients[idx].name);
}

void delete_client(int id) {
  int idx = find_client_by_id(id);
  if (idx < 0) {
    printf("error: unknown id\n");
    exit(1);
  }
  clients[idx].sockfd = -1;
  printf("client[%d] deleted (id=%d,name=%s)\n", idx, id, clients[idx].name);
}

void name_client(int id, char *newname) {
  char oldname[MAX_NAME];
  int idx = find_client_by_id(id);
  if (idx < 0) {
    printf("error: unknown id\n");
    exit(1);
  }
  strcpy(oldname, clients[idx].name);
  strcpy(clients[idx].name, newname);
  printf("client[%d] changed its name (%s->%s)\n", idx, oldname, newname);
}

///////////////////////////////////////////////
// system programming 
///////////////////////////////////////////////

void signal_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    // shutdown server gracefully.
    // close all socket descriptors.
    exit(0);
  }
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
void *receive_Thread(void *arg) 
{
  char buf[MAX_BUFFER], buffer[MAX_BUFFER];
  int id= (int) arg;
  int sockfd, nread, nconsumed, totread = 0;
  char *cpos, *npos, *rpos;
  int retval = 1;

  
  sockfd = clients[find_client_by_id(id)].sockfd;
  memset(buffer, 0, sizeof(buffer));

  while (1) 
  {
    memset(buf, 0, MAX_BUFFER);
    nread = read(sockfd, buf, sizeof(buf));
    if (nread < 0) {
      printf("thread[%d]: error: read(): %s\n", id, strerror(errno));
      pthread_exit((void *)&retval);
    }
    else if (nread == 0) {
      printf("thread[%d]: socket closed\n", id);
      pthread_exit((void *)&retval);
    }
    // else if (nread > 0)
    //print_string(buf);
    //printf("%s", buf);

    if (sizeof(buffer) - strlen(buffer) <= strlen(buf)) {
      printf("thread[%d]: too small buffer\n", id);
      pthread_exit((void *)&retval);
    }

    //strcpy(buffer, "a\nbb\nccc"); totread = 8;
    strcat(buffer, buf);
    totread += nread;
    cpos = buffer;
    while ((npos = (char *) memchr(cpos, '\n', totread)) != NULL) {
      *npos = 0; // replace '\n' with NULL
      if ((rpos = (char *) memchr(cpos, '\r', totread)) != NULL)
	*rpos = 0; // replace '\r' with NULL
      printf("thread[%d]: line = %s\n", id, cpos);
      if (process_msg(id, cpos) != 0) pthread_exit((void *)&retval);
      nconsumed = npos - cpos + 1;
      totread -= nconsumed;
      cpos = npos + 1;
    }
    strcpy(buf, cpos);
    memset(buffer, 0, MAX_BUFFER);
    strcpy(buffer, buf);
    if (strlen(buffer) > 0) {
      printf("thread[%d]: buffer = %s (not completed)\n", id, buffer);
    }
  }
}
///////////////////////////////////////////////
// recv_thread routine for android app
///////////////////////////////////////////////
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


void *recv_main_w_protobuf(void *arg) 
{
  int *sockfd=(int *)arg;
  char buffer[4];
  int bytecount=0;
  string output,pl;
  Umsg *umsg;
  int check = 0;

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
      _recv_process(umsg);
      switch(umsg->_to()) {
      case APP :
	if(umsg->_varid() == 15) {
	  send_thread[TELNET]->PostMsg((const Umsg *) umsg);
	} else {
	  send_thread[APP]->PostMsg((const Umsg *) umsg);
	}
	break;
      case ECAT : ecat_thread->PostMsg((const Umsg *) umsg);
	break;
      case OBD2 : obd2_thread->PostMsg((const Umsg *) umsg);
	break;
      }
    }
}


///////////////////////////////////////////////
// main function
///////////////////////////////////////////////

int main(int argc, char *argv[]) 
{
  int c, flag_help = 0;
  char ipaddr[20] = SERVER_IPADDR;
  int portno = SERVER_PORTNO;
  int halfsd, fullsd; // socket descriptors
  struct sockaddr_in sockaddr;
  pthread_t tid;
  
  while( (c = getopt(argc, argv, "hi:p:")) != -1) 
  {
    switch(c) 
    {
        case 'h': flag_help = 1; break;
        case 'i': memcpy(ipaddr, optarg, strlen(optarg)); break;
        case 'c': portno = atoi(optarg); break;
        default:  printf("unknown option : %c\n", optopt); break;
    }
  }
  if (flag_help == 1) 
  {
    printf("usage: %s [-h] [-i ipaddr] [-p portno] \n", argv[0]);
    exit(1);
  }
  
  printf("server address = %s:%d\n", ipaddr, portno);

  signal(SIGINT,  signal_handler);
  signal(SIGTERM, signal_handler);

  init_clients();
  halfsd = startup_server(ipaddr, portno);

  while (1) {
    int len = sizeof(sockaddr);
    fullsd = accept(halfsd, (struct sockaddr *)&sockaddr, (socklen_t *)&len);
    if (fullsd < 0) 
    {
      printf("error: accept(): %s\n", strerror(errno));
      exit(1);
    }

    if (nclients == MAX_CLIENTS) 
    {
      printf("error: max clients reached\n");
      close(fullsd);
      sleep(60); // wait for a thread to exit for 1 minute
      continue;
    }
    else
    {
        if(
    // register a newly connected client
    add_client(client_id, fullsd, sockaddr);
    nclients++;

    // create a thread to service the client
    if (pthread_create(&tid, NULL, &receive_Thread, (void *)client_id) < 0) 
    {
      printf("error: pthread_create(): %s\n", strerror(errno));
      close(fullsd);
      continue;
    }
    pthread_detach(tid); // make pthread_join() unnecessary

    client_id++; // increment client_id for the next incoming client
  }    
  
  return 0;
}


