#include "global.h"


//      name,           varid,value,timestamp,min,   max,   version
Var obd2_var[]=
{
    {"current velocity", 101,   0,      0,     0,     0,        0},
    {"obd_state",        110,   0,      0,     1,     4,        0}
};
Var ecat_var[]=
{
    {"kpt",               1,  600,      0,     0,   1000,       0},
    {"kit",               2,   25,      0,     0,   1000,       0},
    {"kdt",               3,  300,      0,     0,   1000,       0},
    {"kpb",               4, 3000,      0,     0,   4000,       0},
    {"kib",               5,  250,      0,     0,   1000,       0},
    {"kdb",               6, 2000,      0,     0,   3000,       0},
    {"spos",              9,    0,      0,     0,   500000,     0},
    {"cycle",            22,    5,      0,     0,   1000,       0},
    {"stair",            23,    3,      0,     0,   1000,       0},
    {"vel_diff",         24,    3,      0,     0,   1000,       0},
    {"tvel",              7,    0,      0,     0,     30,       0},
    {"EtherCAT_State",   27,    0,      0,     1,      4,       0},
    {"Motor_State",      28,    0,      0,     5,      9,       0},
};
