# Set compiler to use
GG=g++
CC=c++
CFLAGS=-g -I. -I/opt/etherlab/include -fpermissive -std=c++11 -Wwrite-strings
LDFLAGS= -lpthread
PROTO_FLAGS= `pkg-config --cflags --libs protobuf`
LDFLAGS_BOOST=-L/usr/local/lib -lboost_system -lboost_thread
DEBUG=0


all:: protoc_middleman Client_cpp.exe VCFServer_cpp.exe 

proto: protoc_middleman

cpp: myVCFServ.o 

protoc_middleman: umsg.proto
	protoc -I=. --cpp_out=. --java_out=. ./umsg.proto
	@touch protoc_middleman

VCFServer_cpp.exe: ecat.o myVCFServ.o HybridAutomata.o umsg.pb.o obd2.o 
	$(GG) -o $@ $^ $(CFLAGS) /opt/etherlab/lib/libethercat.a $(LDFLAGS) $(PROTO_FLAGS)

Client_cpp.exe: Client.o
	$(CC) $^ umsg.pb.cc -o  $@ $(PROTO_FLAGS)
 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -f *.exe *.o *~ *.stackdump *_middleman .*.swo .*.swp
