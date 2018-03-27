#pragma once
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
#include <pthread.h>

#include "VThread.h"
#include "ecat.h"
#include "obd2.h"
#include "global.h"
#include "HybridAutomata.h"

#include <fstream>
#include <google/protobuf/util/time_util.h>
#include "umsg.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace vcf;
using namespace google::protobuf::io;

extern Var obd2_var[OBD2_MAX];
extern Var ecat_var[ECAT_MAX];