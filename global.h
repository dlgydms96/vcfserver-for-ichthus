#pragma once
struct OBD2_CurrentVelocity
{
    const char *name = "current velocity";
    const unsigned int varID = 101;
    int value = 0;
    int timestamp = 0;
    const unsigned int version = 0;
};

struct OBD2_STATE
{
    const char *name = "obd_state";
    const unsigned int varID = 110;
    int value = 0;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 3;
    const unsigned int version = 0;
};
struct ECAT_kpt
{
    const char *name = "kpt";
    const unsigned int varID = 1;
    int value = 600;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_kit
{
    const char *name = "kit";
    const unsigned int varID = 2;
    int value = 25;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_kdt
{
    const char *name = "kdt";
    const unsigned int varID = 3;
    int value = 300;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_kpb
{
    const char *name = "kpb";
    const unsigned int varID = 4;
    int value = 3000;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 4000;
    const unsigned int version = 0;
};
struct ECAT_kib
{
    const char *name = "kib";
    const unsigned int varID = 5;
    int value = 250;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_kdb
{
    const char *name = "kdb";
    const unsigned int varID = 6;
    int value = 2000;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 3000;
    const unsigned int version = 0;
};
struct ECAT_SteeringPosition
{
    const char *name = "spos";
    const unsigned int varID = 9;
    int value = 0;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 500000;
    const unsigned int version = 0;
};
struct ECAT_cycle
{
    const char *name = "cycle";
    const unsigned int varID = 22;
    int value = 5;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_stair
{
    const char *name = "stair";
    const unsigned int varID = 23;
    int value = 3;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};
struct ECAT_veldiff
{
    const char *name = "vel_diff";
    const unsigned int varID = 24;
    int value = 3;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 1000;
    const unsigned int version = 0;
};

struct ECAT_TargetVelocity
{
    const char *name = "tvel";
    const unsigned int varID = 7;
    int value = 0;
    int timestamp = 0;
    const unsigned int min = 0;
    const unsigned int max = 30;
    const unsigned int version = 0;
};

struct ECAT_STATE
{
    const char *name = "EtherCAT_State";
    const unsigned int varID = 27;
    int value = 0;
    int timestamp = 0;
    const unsigned int min = 1;
    const unsigned int max = 4;
    const unsigned int version = 0;
};
struct ECAT_Motor_STATE
{
    const char *name = "Motor_State";
    const unsigned int varID = 28;
    int value = 0;
    int before_value = 0;
    int timestamp = 0;
    const unsigned int min = 7;
    const unsigned int max = 9;
    const unsigned int version = 0;
};

struct ECAT
{
    struct ECAT_kpt kpt;
    struct ECAT_kit kit;
    struct ECAT_kdt kdt;
    struct ECAT_kpb kpb;
    struct ECAT_kib kib;
    struct ECAT_kdb kdb;
    struct ECAT_SteeringPosition spos;
    struct ECAT_cycle cycle;
    struct ECAT_stair stair;
    struct ECAT_veldiff vel_diff; //23
    struct ECAT_TargetVelocity tvel; //6
    struct ECAT_STATE ecat_state;
    struct ECAT_Motor_STATE motor_state;
};
struct OBD2
{
    struct OBD2_CurrentVelocity vel;
    struct OBD2_STATE obd2_state;
};

extern struct ECAT ecat_var;
extern struct OBD2 obd2_var;

enum
{
    INIT,
    ECAT,
    OBD2
};
enum
{
    ECAT_START,
    ECAT_UP,
    ECAT_ON,
    ECAT_OFF,
    ECAT_DOWN,
    ECAT_FINISH
};
enum
{
    MOTOR_START, //0
    MOTOR_ERROR, //1
    MOTOR_UP,    //2
    MOTOR_ON,    //3
    MOTOR_READY, //4
    MOTOR_AUTO,   //5
    MOTOR_PID,   
    MOTOR_RESET,  //7
    MOTOR_REGULAR_STOP,  //8
    MOTOR_EMERGENCY_STOP, //9
    MOTOR_FINISH,
};
enum
{
    OBD2_START,
    OBD2_UP,
    OBD2_ON,
    OBD2_OFF,
    OBD2_DOWN,
    OBD2_FINISH
};
//enum {NON, GET, SET};