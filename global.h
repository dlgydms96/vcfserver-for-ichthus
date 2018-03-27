#pragma once
#define ECAT_MAX ECAT_MOTOR_STATE+1
#define OBD2_MAX OBD2_STATE+1
typedef struct variables
{
    const char *name;
    const unsigned int varID;
    int value;
    int timestamp;
    const unsigned int min;
    const unsigned int max;
    const unsigned int version;
} Var;

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
    MOTOR_FINISH
};
enum
{
    PID_START,
    PID_CALC,
    PID_ACCEL,
    PID_BRAKE,
    PID_FINISH
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
enum
{
    OBD2_VEL,
    OBD2_STATE
};
enum
{
    ECAT_KPT,
    ECAT_KIT,
    ECAT_KDT,
    ECAT_KPB,
    ECAT_KIB,
    ECAT_KDB,
    ECAT_SPOS,
    ECAT_CYCLE,
    ECAT_STAIR,
    ECAT_VEL_DIFF,
    ECAT_TVEL,
    ECAT_STATE,
    ECAT_MOTOR_STATE
};
