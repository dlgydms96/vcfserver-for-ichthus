#pragma once
//#include <pthread.h>
#define ECAT_START 1
#define OBD2_START 101
#define APP_START 1001

typedef struct ECAT
{
  struct Target_Velocity tvel;
  struct EtherCAT_STATE ecat_state;
};
typedef struct OBD2
{
    struct OBD2_STATE obd2_state;
}
typedef struct OBD2_STATE;
{
    const char * name = "obd_state";
    const unsigned int varID = 110;
    int value = 0;
    int timestamp = 0;
    const unsigned int version = 0;
}

typedef struct Target_Velocity
{
    const char * name = "tvel";
    const unsigned int varID = 7;
    int value = 0;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 30;
    const unsigned int version = 0;
};

typedef struct EtherCAT_STATE
{
    const char * name = "EtherCAT_State";
    const unsigned int varID = 27;
    int value = -1;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 3;
    const unsigned int version = 0;
};



enum {INIT, ECAT, OBD2};
enum {ECAT_UP,ECAT_ON, ECAT_OFF, ECAT_DOWN};
enum {OBD2_UP,OBD2_ON, OBD2_OFF, OBD2_DOWN};
//enum {NON, GET, SET};

//extern int segNo;
//extern Var ecat_var[14];
//extern Var obd2_var[10];
