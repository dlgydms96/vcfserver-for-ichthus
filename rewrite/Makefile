# Set compiler to use
GG=g++
CC=c++
CFLAGS=-g -I. -fpermissive -std=c++11
LDFLAGS= -lpthread
PBCC_FLAGS= umsg.pb.cc
PROTO_FLAGS= `pkg-config --cflags --libs protobuf`
DEBUG=0


all:: protoc_middleman javas_middleman javac_middleman Client_cpp.exe VCFServ_cpp.exe Client_java.exe Server_java.exe

proto: protoc_middleman

cpp: Client_cpp.exe Server_cpp.exe 

protoc_middleman: umsg.proto
	protoc -I=. --cpp_out=. --java_out=. ./umsg.proto
	@touch protoc_middleman
javac_middleman: Client.java protoc_middleman
	javac -cp $$CLASSPATH Client.java com/rubicom/vcf/UmsgProtos.java
	@touch javac_middleman
javas_middleman: Server.java protoc_middleman
	javac -cp $$CLASSPATH Server.java com/rubicom/vcf/UmsgProtos.java
	@touch javas_middleman
Server_cpp.exe: myVCFServ.cpp 
	$(GG) -o $@ $^ $(PBCC_FLAGS) $(CFLAGS) $(LDFLAGS) $(PROTO_FLAGS)
Client_cpp.exe: Client.cc
	$(CC) $^ $(PBCC_FLAGS) -o $@ $(PROTO_FLAGS)
Client_java.exe: javac_middleman
	@echo "Writing shortcut script Client_java.exe..."
	@echo '#! /bin/sh' > Client_java.exe
	@echo 'java -classpath .:$$CLASSPATH Client "$$@"' >> Client_java.exe
	@chmod +x Client_java.exe
Server_java.exe: javas_middleman
	@echo "Writing shortcut script Server_java.exe..."
	@echo '#! /bin/sh' > Server_java.exe
	@echo 'java -classpath .:$$CLASSPATH Server "$$@"' >> Server_java.exe
	@chmod +x Server_java.exe
clean: 
	rm -f *.exe *.o *~ *.stackdump
