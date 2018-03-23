#pragma once

#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "global.h"


#define OBD_USB_PORT "/dev/ttyACM0"
#define WAIT_OBD(str_size) usleep((str_size + 25) * 100)

enum {OBD_DISCONNECT, OBD_INIT, OBD_ON, OBD_OFF, OBD_ERROR};
enum {FALSE, TRUE};

typedef termios Termios;

typedef struct _obd_ {
	int state;
	int next_state;
	Termios options;
	int rdlen;
	int fd;
	int vel;
} OBD;


extern int nobd2_var;
extern pthread_mutex_t glob_mutex;

void obd2_up();
void obd2_on();
void *obd_cyclic(void *name);
void obd2_off();
void obd2_down();
void manage_obd_state(int case_num);
void print_obd_state(char *str, int state);
