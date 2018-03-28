#include "obd2.h"
// extern Var obd2_var[];

char *AT_RESET = "AT Z\r";
char *AT_SP = "AT SP 0\r";
//char * VEHICLE_SPEED = "01 0D\r";
//char * CARRIAGE_RETURN = "\r";

OBD obd;
pthread_t obd_cyclic_thread;

/*Init OBD II*/
void obd2_up()
{
  char buf[128] = {}; //buffer for OBD data

  obd.fd = open(OBD_USB_PORT, O_RDWR | O_NOCTTY | O_SYNC);
  if (obd.fd < 0)
  {
    printf("OBD Open Error\n");
  }

  bzero(&obd.options, sizeof(obd.options));

  obd.options.c_cflag |= B38400 | CS8 | CLOCAL | CREAD | IGNPAR;
  obd.options.c_cflag &= ~CSIZE & ~PARENB & ~CSTOPB & ~CRTSCTS;

  tcflush(obd.fd, TCIFLUSH);
  tcsetattr(obd.fd, TCSANOW, &obd.options);

  if (write(obd.fd, AT_RESET, sizeof(AT_RESET)) < sizeof(AT_RESET))
  {
    printf("OBD AT_RESET Write Error\n");
  }
  WAIT_OBD(sizeof(AT_RESET));

  obd.rdlen = read(obd.fd, buf, sizeof(buf));
  if (obd.rdlen > 0)
  {
    buf[obd.rdlen] = '\0';
    printf("%s\n", buf);
  }
  else
  {
    printf("OBD Read Error\n");
  }

  //  if(write(obd.fd, AT_SP, sizeof(AT_SP)) < sizeof(AT_SP)) {
  //  printf("OBD AT_SP Write Error\n");
  //  return FALSE;
  // }
  //WAIT_OBD(sizeof(AT_SP));

  //obd.rdlen = read(obd.fd, buf, sizeof(buf));
  //if(obd.rdlen > 0) {
  //  buf[obd.rdlen] = '\0';
  //  printf("%s\n", buf);
  //} else {
  //  printf("OBD Read Error\n");
  //  return FALSE;
  // }

  obd.state = OBD_INIT;
}

void obd2_on()
{
  obd.state = OBD_ON;
  printf("OBD_ON called\n");
  if (pthread_create(&obd_cyclic_thread, 0, obd_cyclic, NULL))
  {
    printf("OBD thread err\n");
  }
}

void *obd_cyclic(void *name)
{
  int rdlen, i, j;
  char buf[128] = {0};
  char change_buf[128] = {0};
  int response;
  int obd_log_fd;
  char VEHICLE_SPEED[] = "01 0D\r";
  char CARRIAGE_RETURN[] = "\r";
  obd.state = OBD_ON;

  obd_log_fd = open("obd_log1.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
  if (obd_log_fd < 0)
  {
    printf("OBD Log Err\n");
  }
  if (write(obd.fd, VEHICLE_SPEED, sizeof(VEHICLE_SPEED)) < sizeof(VEHICLE_SPEED))
  {
    printf("OBD VEHICLE_SPEED Write Error\n");
    obd.state = OBD_ERROR;
  }
  WAIT_OBD(sizeof(VEHICLE_SPEED));

  while (1)
  {
    if (write(obd.fd, CARRIAGE_RETURN, sizeof(CARRIAGE_RETURN)) < sizeof(CARRIAGE_RETURN))
    {
      printf("OBD CARRIAGE_RETURN Write Error\n");
      obd.state = OBD_ERROR;
    }
    usleep(sizeof(CARRIAGE_RETURN) * 100);
    rdlen = read(obd.fd, buf, sizeof(buf));
    if (rdlen > 0)
    {
      for (i = 6, j = 0; i < rdlen; ++i, ++j)
      {
        if (buf[i] != ' ')
        {
          change_buf[j] = buf[i];
        }
        else
        {
          --j;
        }
      }
      change_buf[j] = '\0';
      sscanf(change_buf, "%x", &obd.vel);
      obd2_var[OBD2_VEL].value = obd.vel;
      printf("obd2 vel : %d\n",obd2_var[OBD2_VEL].value);
      obd2_var[OBD2_VEL].timestamp++;
      sprintf(buf, "%d\n", obd.vel);
      write(obd_log_fd, buf, strlen(buf));
    }
    else
    {
      sprintf(buf, "OBD Vel read Error\n");
      write(obd_log_fd, buf, strlen(buf));
      obd.state = OBD_ERROR;
    }
  }
}

void obd2_off()
{
  obd.state = OBD_OFF;
  printf("OBD_OFF called\n");
  pthread_cancel(obd_cyclic_thread);
  pthread_join(obd_cyclic_thread, NULL);
}

void obd2_down()
{
  obd.state = OBD_DISCONNECT;
  printf("OBD_DOWN called\n");
  close(obd.fd);
}

void manage_obd_state(int case_num)
{
  if (case_num == obd.next_state)
  {
    ++(obd.next_state);

    switch (case_num)
    {
    case OBD_DISCONNECT:
      obd2_down();
      break;

    case OBD_INIT:
      obd2_up();
      break;

    case OBD_ON:
      obd2_on();
      break;

    case OBD_OFF:
      obd2_off();
      obd.next_state = 0;
      break;
    }
  }
  else
  {
    print_obd_state("Current State : ", obd.state);
    print_obd_state("Next State : ", obd.next_state);
    print_obd_state("You Tried : ", case_num);
  }

  return;
}

void print_obd_state(char *str, int state)
{
  printf("%s", str);

  switch (state)
  {
  case OBD_DISCONNECT:
    printf("OBD DISCONNECT\n");
    break;
  case OBD_INIT:
    printf("OBD INIT\n");
    break;
  case OBD_ON:
    printf("OBD ON\n");
    break;
  case OBD_OFF:
    printf("OBD OFF\n");
    break;
  case OBD_ERROR:
    printf("OBD ERROR\n");
    break;
  }

  return;
}
