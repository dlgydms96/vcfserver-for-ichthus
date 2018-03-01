#pragma once

#include "global.h"
#include "ecrt.h"
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

#define GEAR_SLAVE		0
#define HANDLE_SLAVE		1
#define ACCEL_SLAVE		2
#define BRAKE_SLAVE		3

#define MAXON_EPOS 0x000000fb, 0x64400000

#define SLAVE_NUM 4

#define MASTER_STATE_INIT	0x00000001
#define MASTER_STATE_PREOP	0x00000002
#define MASTER_STATE_SAFEOP	0x00000004
#define MASTER_STATE_OP		0x00000008

#define GEAR_MAX_POS			1000
#define BRAKE_MAX_POS			206046	
#define ACCEL_MAX_POS			55000
#define HANDLE_MAX_PLUS_POS		237478
#define HANDLE_MAX_MINUS_POS	-237478

#define MASK_STATUSWORD			0x417F
#define FLAGS_SWITCHON_DISABLED	0x0140
#define FLAGS_READYTO_SWITCHON	0x0121
#define FLAGS_OPERATION_ENABLE	0x0137
#define FLAGS_ERROR				0x0108	
#define FLAGS_TARGET_REACHED	0x0400

#define CSP_PDO_COUNT 14
#define PPM_PDO_COUNT 16

#define NANOS (1000000000LL)
#define TASK_FREQUENCY	1000
#define ONE_MILLION		1000000
#define ONE_THOUSAND	1000
#define NSEC_PER_SEC (1000000000L)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC +	\
		       (B).tv_nsec - (A).tv_nsec)
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define US2NS(T) (T * ONE_THOUSAND)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)

typedef struct _master_info {
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

typedef struct _motor_info {
  int no;
  int target_pos;
  int act_pos;
  uint16_t status;
} Motor_info;

typedef struct{
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

typedef struct{
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

typedef struct{
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

typedef struct{
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
} offset_CSMS;//Cyclic Synchronous Modes(CSP/CSV/CST)

typedef struct{
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
} offset_PMS;//Profile Modes(PPM/PVM/IPM)


enum {MOTOR_ERROR, MOTOR_UP, MOTOR_READY, MOTOR_ON, MOTOR_PID, MOTOR_AUTO, MOTOR_RESET, MOTOR_MANUAL, MOTOR_QUICK_STOP, MOTOR_EMERGENCY_STOP};
enum {OP_INIT, OP_HOMING, OP_ACT};
enum {TARGET_REACHED, TARGET_NOT_REACHED};
enum {FIRST, SECOND, THIRD, FORTH};
enum {WAIT, BUSY, IDLE};
enum {CHECK_ACC_READY, CHECK_ACC_ACTIVE};
enum {ECAT_UP, ECAT_ON, ECAT_OFF, ECAT_DOWN};
enum {ACCELERATOR, DECELERATOR};
enum {FLATLAND, UP_HILL, DOWN_HILL};
enum {P, R, N, D, X};

extern Var obd2_var[10];
extern Var ecat_var[14];
extern int motor_fsm_state;

void ecat_up();
void ecat_on();
void ecat_off();
void ecat_down();
void reset_pid();
void init_i_err_buf(int caching);
void i_err_buf_measure();
void init_slope_measure();
void get_slave_state(int index);
void check_master_state();
void check_domain_state();
struct timespec timespec_add(struct timespec time1, struct timespec time2);
#if SOD_ACCESS
void read_sdo(void);
void write_sdo(ec_sdo_request_t *sdo, uint8_t *data, size_t size);
#endif
