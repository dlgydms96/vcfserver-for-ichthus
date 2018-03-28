#pragma once
//check list 0328 in SNU
/*//////////////////////
pullover brake_start_pos = 5000?
pid_calc || pid_calc2 ?


//////////////////////*/

#include "global.h"
#include "ecrt.h"
#include "HybridAutomata.h"
#include "bitop.h"
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/times.h>
#include <signal.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>

#define GEAR_SLAVE 0
#define HANDLE_SLAVE 0
#define ACCEL_SLAVE 2
#define BRAKE_SLAVE 3

#define MAXON_EPOS 0x000000fb, 0x64400000

#define SLAVE_NUM 4

#define MASTER_STATE_INIT 0x00000001
#define MASTER_STATE_PREOP 0x00000002
#define MASTER_STATE_SAFEOP 0x00000004
#define MASTER_STATE_OP 0x00000008

#define GEAR_MAX_POS 1000
#define BRAKE_MAX_POS 191215
#define ACCEL_MAX_POS 55000
#define HANDLE_MAX_PLUS_POS 237478
#define HANDLE_MAX_MINUS_POS -237478

#define MASK_STATUSWORD 0x417F
#define FLAGS_SWITCHON_DISABLED 0x0140
#define FLAGS_READYTO_SWITCHON 0x0121
#define FLAGS_OPERATION_ENABLE 0x0137
#define FLAGS_ERROR 0x0108
#define FLAGS_TARGET_REACHED 0x0400

#define CSP_PDO_COUNT 14
#define PPM_PDO_COUNT 16

#define NANOS (1000000000LL)
#define TASK_FREQUENCY 1000
#define ONE_MILLION 1000000
#define ONE_THOUSAND 1000
#define NSEC_PER_SEC (1000000000L)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
                       (B).tv_nsec - (A).tv_nsec)
#define TIMESPEC2NS(T) ((uint64_t)(T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define US2NS(T) (T * ONE_THOUSAND)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)

typedef struct _master_info
{
  ec_master_t *master;
  ec_master_state_t master_state;
  ec_domain_t *domain;
  ec_domain_state_t domain_state;
  uint8_t *domain_pd;
  ec_slave_config_t *sc_epos[SLAVE_NUM];
  ec_slave_config_state_t slave_state[SLAVE_NUM];
  pthread_t cyclic_thread;
  int ecat_state;
} Master_info;

typedef struct _motor_info
{
  int no;
  int target_pos;
  int act_pos;
  uint16_t status;
} Motor_info;

typedef struct
{
  unsigned int ctrl_word;
  unsigned int target_position;
  unsigned int pos_offset;
  unsigned int vel_offset;
  unsigned int toq_offset;
  unsigned int mod_op;
  unsigned int dig_out;
  unsigned int status_word;
  unsigned int act_velocity;
  unsigned int act_position;
  unsigned int act_toq;
  unsigned int mod_op_dis;
  unsigned int dig_state;
} offset_CSP;

typedef struct
{
  unsigned int ctrl_word;
  unsigned int target_vel;
  unsigned int vel_offset;
  unsigned int toq_offset;
  unsigned int mod_op;
  unsigned int dig_out;
  unsigned int status_word;
  unsigned int act_velocity;
  unsigned int act_position;
  unsigned int act_toq;
  unsigned int mod_op_dis;
  unsigned int dig_state;
} offset_CSV;

typedef struct
{
  unsigned int ctrl_word;
  unsigned int target_toq;
  unsigned int toq_offset;
  unsigned int mod_op;
  unsigned int dig_out;
  unsigned int status_word;
  unsigned int act_velocity;
  unsigned int act_position;
  unsigned int act_toq;
  unsigned int mod_op_dis;
  unsigned int dig_state;
} offset_CST;

typedef struct
{
  unsigned int ctrl_word;
  unsigned int target_pos;
  unsigned int target_vel;
  unsigned int target_toq;
  unsigned int pos_offset;
  unsigned int vel_offset;
  unsigned int toq_offset;
  unsigned int mod_op;
  unsigned int dig_out;
  unsigned int status_word;
  unsigned int act_velocity;
  unsigned int act_position;
  unsigned int act_toq;
  unsigned int mod_op_dis;
  unsigned int dig_state;
} offset_CSMS; //Cyclic Synchronous Modes(CSP/CSV/CST)

typedef struct
{
  unsigned int ctrl_word;
  unsigned int target_pos;
  unsigned int target_vel;
  unsigned int pro_accel;
  unsigned int pro_decel;
  unsigned int pro_vel;
  unsigned int mod_op;
  unsigned int dig_out;
  unsigned int status_word;
  unsigned int act_velocity;
  unsigned int act_position;
  unsigned int act_cur;
  unsigned int act_fwE;
  unsigned int mod_op_dis;
  unsigned int dig_state;
} offset_PMS; //Profile Modes(PPM/PVM/IPM)

enum
{
  ACCELERATOR,
  DECELERATOR
};

enum
{
  P,
  R,
  N,
  D,
  X
};

extern Var obd2_var[OBD2_MAX];
extern Var ecat_var[ECAT_MAX];
void ecat_up();
void ecat_on();
void ecat_off();
void ecat_down();
void reset_pid();
void get_slave_state(int index);
void check_master_state();
void check_domain_state();
void preprocessing_ecat();
void postprocessing_ecat();
void motor_error();
void motor_up();
void motor_on();
void motor_ready();
void motor_auto();
void HA_EcatCyclicThread();
class PID_CALC2ACCEL :public Condition
{
  public:
  bool check(HybridAutomata *HA);
};
class PID_CALC2BRAKE :public Condition
{
  public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ALL2ERROR : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2UP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2ON : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2AUTO : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2PID : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2RESET : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2REGULAR_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ERROR2EMERGENCY_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
//////////////////////////////////////////////////////
class MOTOR_UP2UP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_UP2ON : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ON2ON : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_ON2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_READY2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_READY2AUTO : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_READY2RESET : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_AUTO2AUTO : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_AUTO2PID : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_AUTO2RESET : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_RESET2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_RESET2RESET : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_AUTO2REGULAR_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_REGULAR_STOP2REGULAR_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_REGULAR_STOP2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_AUTO2EMERGENCY_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_EMERGENCY_STOP2EMERGENCY_STOP : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
class MOTOR_EMERGENCY_STOP2READY : public Condition
{
public:
  bool check(HybridAutomata *HA);
};
struct timespec timespec_add(struct timespec time1, struct timespec time2);
#if SOD_ACCESS
void read_sdo(void);
void write_sdo(ec_sdo_request_t *sdo, uint8_t *data, size_t size);
#endif
