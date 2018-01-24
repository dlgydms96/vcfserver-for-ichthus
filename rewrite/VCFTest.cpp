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
    /* cout<<"Message is "<<umsg->DebugString()<<endl;
     cout<<"_seqno : "<<umsg->_seqno()<<endl;
     cout<<"_from : "<<umsg->_from()<<endl;
     cout<<"_to : "<<umsg->_to()<<endl;
     cout<<"_cmd : "<<umsg->_cmd()<<endl;
     cout<<"_varID : "<<umsg->_varid()<<endl;
     cout<<"_value : "<<umsg->_value()<<endl;
     cout<<"_result : "<<umsg->_result()<<endl;
     cout<<"_timestamp : "<<umsg->_timestamp()<<endl;
     cout<<"_msg : "<<umsg->_msg()<<endl;
     */
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


void send_handler(VThread *t, ThreadMsg *msg) 
{
  const Umsg* umsg = static_cast<const Umsg*>(msg->msg);
  cout<<"size after serilizing is "<<umsg->ByteSize()<<endl;
  int siz = umsg->ByteSize()+1;
  char *pkt = new char [siz];
  google::protobuf::io::ArrayOutputStream aos(pkt,siz);
  CodedOutputStream *coded_output = new CodedOutputStream(&aos);
  coded_output->WriteVarint32(umsg->ByteSize());
  umsg->SerializeToCodedStream(coded_output);
/*
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
  */
}


