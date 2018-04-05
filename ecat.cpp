#include "ecat.h"

////////////for pull over////////////////
int glob_vel_timestamp = obd2_var[OBD2_VEL].timestamp;
int brake_pos_p_tick = 30;
int brake_start_pos = 50000;
int count = 0;

///////////for MOTOR_ERROR///////////
unsigned int before_value;
int motor_err_idx = 0;

////////////for handle///////////
//int count;
int angle_p_p = 58;
int m_angle_p_p = -58;
int p_angle_p_p = 58;
int prev_pos;
int total_pos;
int handle_count;

offset_CSP offset_csp[SLAVE_NUM];
/* Master 0, Slave 0, "EPOS3"
 * Vendor ID:       0x000000fb
 * Product code:    0x64400000
 * Revision number: 0x22000000
 */

/*
 *  [EPOS3's Modes]
 *  <Mod>	<PDO>	<Description>
 *  7		5		Interpolated Position Mode (PVT)
 *  6		-		Homing Mode
 *  3		5		Profile Velocity Mode (PVM)
 *  1		5		Profile Position Mode (PPM)
 *  8		1,4		Cyclic Synchronous Position Mode (CSP)
 * 	9		2,4		Cyclic Synchronous Velocity Mode (CSV)
 *  10		3,4		Cyclic Synchronous Torque Mode (CST)
 */

/*
 *	Recieve PDO 1 Mapping - CSP
 */

ec_pdo_entry_info_t Receive_pdo_1_entries[] = {
    {0x6040, 0x00, 16},
    {0x607a, 0x00, 32},
    {0x60b0, 0x00, 32},
    {0x60b1, 0x00, 32},
    {0x60b2, 0x00, 16},
    {0x6060, 0x00, 8},
    {0x2078, 0x01, 16},
};

/*
 *	Recieve PDO 2 Mapping - CSV
 */
ec_pdo_entry_info_t Receive_pdo_2_entries[] = {
    {0x6040, 0x00, 16},
    {0x60ff, 0x00, 32},
    {0x60b1, 0x00, 32},
    {0x60b2, 0x00, 16},
    {0x6060, 0x00, 8},
    {0x2078, 0x01, 16},
};

/*
 *	Recieve PDO 3 Mapping - CST
 */
ec_pdo_entry_info_t Receive_pdo_3_entries[] = {
    {0x6040, 0x00, 16},
    {0x6071, 0x00, 16},
    {0x60b2, 0x00, 16},
    {0x6060, 0x00, 8},
    {0x2078, 0x01, 16},
};

/*
 *	Recieve PDO 4 Mapping - CSP/CSV/CST
 */
ec_pdo_entry_info_t Receive_pdo_4_entries[] = {
    {0x6040, 0x00, 16},
    {0x607a, 0x00, 32},
    {0x60ff, 0x00, 32},
    {0x6071, 0x00, 16},
    {0x60b0, 0x00, 32},
    {0x60b1, 0x00, 32},
    {0x60b2, 0x00, 16},
    {0x6060, 0x00, 8},
    {0x2078, 0x01, 16},
};

/*
 *	Recieve PDO 5 Mapping - PPM/PVM/IPM
 */
ec_pdo_entry_info_t Receive_pdo_5_entries[] = {
    {0x6040, 0x00, 16},
    {0x607a, 0x00, 32},
    {0x60ff, 0x00, 32},
    {0x6083, 0x00, 32},
    {0x6084, 0x00, 32},
    {0x6081, 0x00, 32},
    {0x6060, 0x00, 8},
    {0x2078, 0x01, 16},
};

/*
 *	Transmit PDO 1 Mapping - CSP
 */
ec_pdo_entry_info_t Transmit_pdo_1_entries[] = {
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x2071, 0x01, 16},
};

/*
 *	Transmit PDO 2 Mapping - CSV
 */
ec_pdo_entry_info_t Transmit_pdo_2_entries[] = {
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x2071, 0x01, 16},
};

/*
 *	Transmit PDO 3 Mapping - CST
 */
ec_pdo_entry_info_t Transmit_pdo_3_entries[] = {
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x2071, 0x01, 16},
};

/*
 *	Transmit PDO 4 Mapping - CSP/CSV/CST
 */
ec_pdo_entry_info_t Transmit_pdo_4_entries[] = {
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x2071, 0x01, 16},
};

/*
 *	Transmit PDO 5 Mapping - PPM/PBM/IPM
 */
ec_pdo_entry_info_t Transmit_pdo_5_entries[] = {
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6078, 0x00, 16},
    {0x20f4, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x2071, 0x01, 16},
    {0x20c4, 0x01, 16},
};

ec_pdo_info_t EPOS3_pdos[] = {
    {0x1600, 7, Receive_pdo_1_entries},
    {0x1601, 6, Receive_pdo_2_entries},
    {0x1602, 5, Receive_pdo_3_entries},
    {0x1603, 9, Receive_pdo_4_entries},
    {0x1604, 8, Receive_pdo_5_entries},
    {0x1a00, 6, Transmit_pdo_1_entries},
    {0x1a01, 6, Transmit_pdo_2_entries},
    {0x1a02, 6, Transmit_pdo_3_entries},
    {0x1a03, 6, Transmit_pdo_4_entries},
    {0x1a04, 8, Transmit_pdo_5_entries},
};

ec_sync_info_t EPOS3_pdo_1_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, EPOS3_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, EPOS3_pdos + 5, EC_WD_DISABLE},
    {0xff}};

ec_sync_info_t EPOS3_pdo_2_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, EPOS3_pdos + 1, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, EPOS3_pdos + 6, EC_WD_DISABLE},
    {0xff}};

ec_sync_info_t EPOS3_pdo_3_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, EPOS3_pdos + 2, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, EPOS3_pdos + 7, EC_WD_DISABLE},
    {0xff}};

ec_sync_info_t EPOS3_pdo_4_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, EPOS3_pdos + 3, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, EPOS3_pdos + 8, EC_WD_DISABLE},
    {0xff}};

ec_sync_info_t EPOS3_pdo_5_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, EPOS3_pdos + 4, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, EPOS3_pdos + 9, EC_WD_DISABLE},
    {0xff}};

Master_info master_info;
Motor_info motor_info[SLAVE_NUM];
int motor_fd;
int pid_fd;

const ec_pdo_entry_reg_t domain1_regs1[] = {
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6040, 0x0, &offset_csp[GEAR_SLAVE].ctrl_word},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x607A, 0x0, &offset_csp[GEAR_SLAVE].target_position},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x60B0, 0x0, &offset_csp[GEAR_SLAVE].pos_offset},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x60B1, 0x0, &offset_csp[GEAR_SLAVE].vel_offset},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x60B2, 0x0, &offset_csp[GEAR_SLAVE].toq_offset},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6060, 0x0, &offset_csp[GEAR_SLAVE].mod_op},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x2078, 0x1, &offset_csp[GEAR_SLAVE].dig_out},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6041, 0x0, &offset_csp[GEAR_SLAVE].status_word},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6077, 0x0, &offset_csp[GEAR_SLAVE].act_toq},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x606C, 0x0, &offset_csp[GEAR_SLAVE].act_velocity},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6064, 0x0, &offset_csp[GEAR_SLAVE].act_position},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x6061, 0x0, &offset_csp[GEAR_SLAVE].mod_op_dis},
    {0, GEAR_SLAVE, MAXON_EPOS, 0x2071, 0x1, &offset_csp[GEAR_SLAVE].dig_state},
    {}};

const ec_pdo_entry_reg_t domain1_regs2[] = {
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6040, 0x0, &offset_csp[HANDLE_SLAVE].ctrl_word},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x607A, 0x0, &offset_csp[HANDLE_SLAVE].target_position},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x60B0, 0x0, &offset_csp[HANDLE_SLAVE].pos_offset},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x60B1, 0x0, &offset_csp[HANDLE_SLAVE].vel_offset},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x60B2, 0x0, &offset_csp[HANDLE_SLAVE].toq_offset},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6060, 0x0, &offset_csp[HANDLE_SLAVE].mod_op},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x2078, 0x1, &offset_csp[HANDLE_SLAVE].dig_out},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6041, 0x0, &offset_csp[HANDLE_SLAVE].status_word},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6077, 0x0, &offset_csp[HANDLE_SLAVE].act_toq},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x606C, 0x0, &offset_csp[HANDLE_SLAVE].act_velocity},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6064, 0x0, &offset_csp[HANDLE_SLAVE].act_position},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x6061, 0x0, &offset_csp[HANDLE_SLAVE].mod_op_dis},
    {0, HANDLE_SLAVE, MAXON_EPOS, 0x2071, 0x1, &offset_csp[HANDLE_SLAVE].dig_state},
    {}};

const ec_pdo_entry_reg_t domain1_regs3[] = {
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6040, 0x0, &offset_csp[ACCEL_SLAVE].ctrl_word},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x607A, 0x0, &offset_csp[ACCEL_SLAVE].target_position},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x60B0, 0x0, &offset_csp[ACCEL_SLAVE].pos_offset},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x60B1, 0x0, &offset_csp[ACCEL_SLAVE].vel_offset},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x60B2, 0x0, &offset_csp[ACCEL_SLAVE].toq_offset},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6060, 0x0, &offset_csp[ACCEL_SLAVE].mod_op},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x2078, 0x1, &offset_csp[ACCEL_SLAVE].dig_out},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6041, 0x0, &offset_csp[ACCEL_SLAVE].status_word},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6077, 0x0, &offset_csp[ACCEL_SLAVE].act_toq},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x606C, 0x0, &offset_csp[ACCEL_SLAVE].act_velocity},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6064, 0x0, &offset_csp[ACCEL_SLAVE].act_position},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x6061, 0x0, &offset_csp[ACCEL_SLAVE].mod_op_dis},
    {0, ACCEL_SLAVE, MAXON_EPOS, 0x2071, 0x1, &offset_csp[ACCEL_SLAVE].dig_state},
    {}};

const ec_pdo_entry_reg_t domain1_regs4[] = {
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6040, 0x0, &offset_csp[BRAKE_SLAVE].ctrl_word},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x607A, 0x0, &offset_csp[BRAKE_SLAVE].target_position},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x60B0, 0x0, &offset_csp[BRAKE_SLAVE].pos_offset},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x60B1, 0x0, &offset_csp[BRAKE_SLAVE].vel_offset},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x60B2, 0x0, &offset_csp[BRAKE_SLAVE].toq_offset},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6060, 0x0, &offset_csp[BRAKE_SLAVE].mod_op},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x2078, 0x1, &offset_csp[BRAKE_SLAVE].dig_out},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6041, 0x0, &offset_csp[BRAKE_SLAVE].status_word},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6077, 0x0, &offset_csp[BRAKE_SLAVE].act_toq},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x606C, 0x0, &offset_csp[BRAKE_SLAVE].act_velocity},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6064, 0x0, &offset_csp[BRAKE_SLAVE].act_position},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x6061, 0x0, &offset_csp[BRAKE_SLAVE].mod_op_dis},
    {0, BRAKE_SLAVE, MAXON_EPOS, 0x2071, 0x1, &offset_csp[BRAKE_SLAVE].dig_state},
    {}};

int get_motor_pos(int slave)
{
  motor_info[slave].act_pos = EC_READ_S32(master_info.domain_pd + offset_csp[slave].act_position);
}

void set_motor_pos(int slave)
{
  EC_WRITE_S32(master_info.domain_pd + offset_csp[slave].target_position, motor_info[slave].target_pos);
}

void init_motor()
{
  int idx;

  for (idx = 0; idx < SLAVE_NUM; ++idx)
  {
    motor_info[idx].no = idx;
    motor_info[idx].target_pos = 0;
    motor_info[idx].act_pos = 0;
    motor_info[idx].status = 0;
  }
}

void oscillation_handle(int target_pos)
{
  static int osci_count = 0;
  static int osci_pos = 25;
  int m_osci_pos = -osci_pos;
  int p_osci_pos = osci_pos;

  switch (osci_count)
  {
  case 0:
    motor_info[HANDLE_SLAVE].target_pos = target_pos - osci_pos;
    osci_count = 1;
    break;
  case 1:
    motor_info[HANDLE_SLAVE].target_pos = target_pos + osci_pos;
    osci_count = 0;
    break;
  }
}

void control_handle() //use global.c
{
  static int total_pos = 0;
  static int prev_pos = 0;
  static int count = 0;
  static int handle_count = 0;
  static int act_motor_target_pos = 0;

  if (total_pos != ecat_var[ECAT_SPOS].value)
  {
    prev_pos = total_pos;
    total_pos = ecat_var[ECAT_SPOS].value;
    if ((prev_pos - total_pos) < 0)
    {
      angle_p_p = m_angle_p_p;
    }
    else
    {
      angle_p_p = p_angle_p_p;
    }

    count = abs((total_pos - prev_pos) / angle_p_p);
    motor_info[HANDLE_SLAVE].target_pos = act_motor_target_pos;
  }

  if (handle_count != count)
  {
    motor_info[HANDLE_SLAVE].target_pos += angle_p_p;
    act_motor_target_pos = motor_info[HANDLE_SLAVE].target_pos;
    ++handle_count;
  }
  else
  {
    handle_count = 0;
    count = 0;
    oscillation_handle(act_motor_target_pos);
  }
}

//for pid control
int kp;
int ki;
int kd;
int dt = 1;
double I_err;
double D_err;
double cur_err;
double prev_err;
double Kp_term;
double Ki_term;
double Kd_term;
double power;
int acc_count;
int dec_count;
int pedal_state = ACCELERATOR;
void my_pid_calc2()
{
  static int kp, ki, kd;
  static int tmp_tvel = ecat_var[ECAT_TVEL].value;
  int vel_from_obd = obd2_var[OBD2_VEL].value;
  static int prev_vel_from_obd = 0;
  //static int pedal_state = ACCELERATOR;
  static int changed_tvel = 0;
  static int keep_state = 0;
  static int delta = ecat_var[ECAT_VEL_DIFF].value; // should be
  static int is_fast = 1;
  static int past_tvel = tmp_tvel;
  static int check_count = 0;
  static int changed_state = 0;
  static int check_vel[4] = {0};
  static int check_vel_count = 0;
  static double very_past_err = 0;
  static int past_err = 0;
  char buf[1024];
  //cout<<"in pid_control"<<endl;
  if (vel_from_obd > 255)
  {
    vel_from_obd = prev_vel_from_obd;
  }
  prev_vel_from_obd = vel_from_obd;
  if (past_tvel != ecat_var[ECAT_TVEL].value) // user changed tvel
  {
    past_tvel = ecat_var[ECAT_TVEL].value;
    changed_tvel = 1;
    keep_state = 1;
    delta = ecat_var[ECAT_VEL_DIFF].value;
    tmp_tvel = vel_from_obd;
    if (ecat_var[ECAT_TVEL].value > vel_from_obd)
    {
      is_fast = 0;
    }
    else
    {
      is_fast = 1;
    }
    changed_state = 0;
    check_vel_count = 0;
  }
  if (changed_tvel)
  {
    if (is_fast)
    {
      if (vel_from_obd < tmp_tvel + (delta - 1)) // is fast now but not that fast (its slower than tmp_tvel + delta)
      {
        if (vel_from_obd > past_tvel + delta) // its still fast
        {
          tmp_tvel = vel_from_obd - delta; //should change tmp_tvel smaller
        }
        else // now its stable
        {
          tmp_tvel = past_tvel;
          changed_tvel = 0;
        }
      }
    }
    else
    {
      if (vel_from_obd > tmp_tvel - (delta - 1)) // its slow but not that slow
      {
        if (vel_from_obd < past_tvel - delta) //its getting very slower
        {
          tmp_tvel = vel_from_obd + delta; // should change tmp_tvel little bit bigger
        }
        else
        {
          tmp_tvel = past_tvel;
          changed_tvel = 0;
        }
      }
    }
  }
  else // if user do not change tvel
  {
    tmp_tvel = past_tvel;
    changed_tvel = 0;
  }
  motor_info[ACCEL_SLAVE].act_pos = get_motor_pos(ACCEL_SLAVE);
  motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE);
  very_past_err = past_err;
  past_err = past_tvel - vel_from_obd;
  cur_err = tmp_tvel - vel_from_obd;
  switch (pedal_state)
  {
  case ACCELERATOR:
    printf("ACCEL\n");
    kp = ecat_var[ECAT_KPT].value;
    ki = ecat_var[ECAT_KIT].value;
    kd = ecat_var[ECAT_KDT].value;
    if (changed_state) // it was DECELARATOR right before(noticed that it is uphill)
    {
      if (tmp_tvel > vel_from_obd) //is slow
      {
        check_vel[check_vel_count] = vel_from_obd;
        ++check_vel_count;
        if (check_vel_count > 3)
        {
          if (check_vel[0] >= check_vel[3])
          {
            I_err = (15000 + 1000 * (check_vel[0] - check_vel[3])) / ecat_var[ECAT_KIT].value;
          }
          changed_state = 0;
          check_vel_count = 0;
        }
      }
      else
      {
        changed_state = 0;
        check_vel_count = 0;
      }
    }
    if (vel_from_obd > tmp_tvel) // is fast
    {
      if (motor_info[ACCEL_SLAVE].act_pos == 0 || vel_from_obd > tmp_tvel + 3) // is very fast
      {
        if (keep_state)
        {
          if (very_past_err > past_err) // but is getting slower
          {
            ++check_count;
          }
          else if (past_err > very_past_err) // its getting more faster!
          {
            if (!check_count)
            {
              --check_count;
            }
          }
          if (check_count > 3) // idle time over, should change state
          {
            check_count = 0;
            keep_state = 0;
          }
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
        }
        else // should change state
        {
          pedal_state = DECELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPB].value;
          ki = ecat_var[ECAT_KIB].value;
          kd = ecat_var[ECAT_KDB].value;
          check_count = 0;
          keep_state = 0;
        }
      }
      else // its fast but under delta, keep this state
      {
        kp = ecat_var[ECAT_KPT].value;
        ki = ecat_var[ECAT_KIT].value;
        kd = ecat_var[ECAT_KDT].value;
      }
    }
    else // is not fast yet
    {
      keep_state = 0;
      check_count = 0;
    }
    break;
  case DECELERATOR:
    printf("DECEL\n");
    kp = ecat_var[ECAT_KPB].value;
    ki = ecat_var[ECAT_KIB].value;
    kd = ecat_var[ECAT_KDB].value;
    if (vel_from_obd < tmp_tvel) // is slow
    {
      if (motor_info[BRAKE_SLAVE].act_pos == 0 || vel_from_obd < tmp_tvel - 3) // is very slow
      {
        if (keep_state)
        {
          if (very_past_err < past_err) // is very slow but getting faster
          {
            ++check_count;
          }
          else if (past_err < very_past_err) // is getting more slower
          {
            if (!check_count)
            {
              --check_count;
            }
          }
          if (check_count > 3) // its very slow should switch state
          {
            check_count = 0;
            keep_state = 0;
          }
        }
        else
        {
          changed_state = 1;
          pedal_state = ACCELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
          check_count = 0;
          keep_state = 0;
        }
      }
    }
    else // is fast
    {
      keep_state = 0;
      check_count = 0;
    }
    break;
  }
  Kp_term = kp * cur_err;
  I_err += cur_err * dt;
  Ki_term = ki * I_err;
  D_err = (prev_err - cur_err) / dt;
  Kd_term = kd * D_err;
  power = Kp_term + Ki_term + Kd_term;
  prev_err = cur_err;
  sprintf(buf, "Kp_term : %lf\tKi_term : %lf\tPower : %lf\tnACC : %d\tnDEC : %d\n", Kp_term, Ki_term, power, acc_count, dec_count);
  // sprintf(buf, "Pedal_state : %d\tMaintain : %d\tCheck_count : %d\tPower : %lf\tpast_tvel : %d\tTvel : %d\tAct_Vel : %d\n", pedal_state, maintain, check_count, power, past_tvel, tmp_tvel, vel_from_obd);
  write(pid_fd, buf, strlen(buf));
  //slope_measure(pedal_state, vel_from_obd); // said not using?
  return;
}

void pid_calc() // not using int maintain1
{
  static int kp, ki, kd;                       //
  static int tvel = ecat_var[ECAT_TVEL].value; //
  int vel_from_obd = obd2_var[OBD2_VEL].value; //
  static int prev_vel_from_obd = 0;            //
  //static int pedal_state = ACCELERATOR;                //
  static int maintain = 0;                                   //
  static int vel_diff = 0;                                   //
  static int vel_diff_count = ecat_var[ECAT_VEL_DIFF].value; //
  static int temp_count = 3;                                 //
  static int act_tvel = tvel;                                //
  static int check_count = 0;                                //
  static int change_pedal = 0;                               //
  static int check_vel[4] = {0};
  static int check_vel_count = 0; //
  static double check_err = 0;    //
  static int act_err = 0;         //
  //int stair = ecat_var[ECAT_STAIR].value;
  char buf[1024];

  if (vel_from_obd > 255)
  {
    vel_from_obd = prev_vel_from_obd;
  }

  prev_vel_from_obd = vel_from_obd;

  if (act_tvel != ecat_var[ECAT_TVEL].value)
  {
    act_tvel = ecat_var[ECAT_TVEL].value;
    tvel = act_tvel;
    maintain = 1;

    if (maintain)
    {
      vel_diff_count = ecat_var[ECAT_VEL_DIFF].value - 1;
      while (1)
      {
        vel_diff = (vel_from_obd - tvel) / (vel_diff_count + 1);
        if (vel_diff > 3)
        {
          ++vel_diff_count;
        }
        else
        {
          break;
        }
      }
    }
    vel_diff_count = ecat_var[ECAT_VEL_DIFF].value - 1;
    temp_count = 3;
    change_pedal = 0;
    check_vel_count = 0;
  }

  if (maintain)
  {
    if (vel_diff)
    {
      if (vel_diff_count)
      {
        tvel = act_tvel + vel_diff * vel_diff_count;
        if (temp_count)
        {
          --temp_count;
        }
        else
        {
          --vel_diff_count;
          temp_count = 3;
        }
      }
      else
      {
        tvel = act_tvel;
        vel_diff = 0;
        vel_diff_count = ecat_var[ECAT_VEL_DIFF].value - 1;
        temp_count = 3;
      }
    }
    else
    {
      tvel = act_tvel;
    }
  }
  else
  {
    tvel = act_tvel;
  }

  motor_info[ACCEL_SLAVE].act_pos = get_motor_pos(ACCEL_SLAVE);
  motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE);

  check_err = act_err;
  act_err = act_tvel - vel_from_obd;
  cur_err = tvel - vel_from_obd;

  switch (pedal_state)
  {
  case ACCELERATOR:
    printf("ACCEL\n");
    kp = ecat_var[ECAT_KPT].value;
    ki = ecat_var[ECAT_KIT].value;
    kd = ecat_var[ECAT_KDT].value;
    if (change_pedal)
    {
      if (vel_from_obd < tvel)
      {
        check_vel[check_vel_count] = vel_from_obd;
        ++check_vel_count;
        if (check_vel_count > 3)
        {
          if (check_vel[0] > check_vel[3])
          {
            I_err = (15000 + 2000 * (check_vel[0] - check_vel[3])) / ecat_var[ECAT_KIT].value;
          }
          change_pedal = 0;
          check_vel_count = 0;
        }
      }
      else
      {
        change_pedal = 0;
        check_vel_count = 0;
      }
    }
    if (cur_err < 0)
    {
      if (motor_info[ACCEL_SLAVE].act_pos == 0 || vel_from_obd > tvel + 3)
      {
        if (maintain)
        {
          if (act_err < check_err)
          {
            ++check_count;
          }
          if (check_count > 3)
          {
            check_count = 0;
            maintain = 0;
          }
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
        }
        else
        {
          pedal_state = DECELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPB].value;
          ki = ecat_var[ECAT_KIB].value;
          kd = ecat_var[ECAT_KDB].value;
          check_count = 0;
          maintain = 0;
        }
      }
      else
      {
        kp = ecat_var[ECAT_KPT].value;
        ki = ecat_var[ECAT_KIT].value;
        kd = ecat_var[ECAT_KDT].value;
      }
    }
    else
    {
      maintain = 0;
      check_count = 0;
    }
    break;
  case DECELERATOR:
    printf("DECEL\n");
    kp = ecat_var[ECAT_KPB].value;
    ki = ecat_var[ECAT_KIB].value;
    kd = ecat_var[ECAT_KDB].value;
    if (cur_err > 0)
    {
      if (motor_info[BRAKE_SLAVE].act_pos == 0 || vel_from_obd < tvel - 3)
      {
        if (maintain)
        {
          if (act_err > check_err)
          {
            ++check_count;
          }
          if (check_count > 3)
          {
            check_count = 0;
            maintain = 0;
          }
        }
        else
        {
          change_pedal = 1;
          pedal_state = ACCELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
          check_count = 0;
          maintain = 0;
        }
      }
    }
    else
    {
      maintain = 0;
      check_count = 0;
    }
    break;
  }

  Kp_term = kp * cur_err;
  I_err += cur_err * dt;
  Ki_term = ki * I_err;
  D_err = (prev_err - cur_err) / dt;
  Kd_term = kd * D_err;

  power = Kp_term + Ki_term + Kd_term;

  prev_err = cur_err;

  sprintf(buf, "Kp_term : %lf\tKi_term : %lf\tPower : %lf\tnACC : %d\tnDEC : %d\n", Kp_term, Ki_term, power, acc_count, dec_count);
  sprintf(buf, "Pedal_state : %d\tMaintain : %d\tCheck_count : %d\tPower : %lf\tAct_tvel : %d\tTvel : %d\tAct_Vel : %d\n", pedal_state, maintain, check_count, power, act_tvel, tvel, vel_from_obd);
  write(pid_fd, buf, strlen(buf));
  //  slope_measure(pedal_state, vel_from_obd);
  return;
}

void pid_calc2()
{
  static int kp, ki, kd;
  static int tvel = ecat_var[ECAT_TVEL].value;
  int vel_from_obd = obd2_var[OBD2_VEL].value;
  static int prev_vel_from_obd = 0;
  //static int pedal_state = ACCELERATOR;
  static int maintain = 0;
  static int maintain1 = 0;
  static int vel_diff = 0;
  static int vel_diff_count = ecat_var[ECAT_CYCLE].value;
  static int act_tvel = tvel;
  static int check_count = 0;
  static int change_pedal = 0;
  static int check_vel[4] = {0};
  static int check_vel_count = 0;
  static double check_err = 0;
  static int act_err = 0;
  int stair = ecat_var[ECAT_STAIR].value;
  char buf[1024];

  //cout<<"in pid_control"<<endl;

  if (vel_from_obd > 255)
  {
    vel_from_obd = prev_vel_from_obd;
  }
  prev_vel_from_obd = vel_from_obd;

  if (act_tvel != ecat_var[ECAT_TVEL].value)
  {
    act_tvel = ecat_var[ECAT_TVEL].value;
    tvel = act_tvel;
    //++ecat_var[ECAT_TVEL].timestampE;

    maintain = 1;

    act_tvel = tvel;
    maintain1 = 1;
    if (maintain)
    {
      vel_diff = ecat_var[ECAT_VEL_DIFF].value;
      tvel = vel_from_obd;
      if (act_tvel - vel_from_obd > 0)
      {
        vel_diff_count = 0;
      }
      else
      {
        vel_diff_count = 1;
      }
    }
    change_pedal = 0;
    check_vel_count = 0;
  } //end of if (act_tvel != ecat_var[ECAT_TVEL].value)

  if (maintain)
  {
    if (vel_diff)
    {
      if (vel_diff_count)
      {
        if (vel_from_obd < tvel + (vel_diff - 1))
        {
          if (vel_from_obd - vel_diff > act_tvel)
          {
            tvel = vel_from_obd - vel_diff;
          }
          else
          {
            tvel = act_tvel;
            vel_diff = 0;
          }
        }
      }
      else
      {
        if (vel_from_obd > tvel - (vel_diff - 1))
        {
          if (vel_from_obd + vel_diff < act_tvel)
          {
            tvel = vel_from_obd + vel_diff;
          }
          else
          {
            tvel = act_tvel;
            vel_diff = 0;
          }
        }
      }
    }
    else
    {
      tvel = act_tvel;
      vel_diff = 0;
    }
  }
  else
  {
    tvel = act_tvel;
    vel_diff = 0;
  }

  motor_info[ACCEL_SLAVE].act_pos = get_motor_pos(ACCEL_SLAVE);
  motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE);

  check_err = act_err;
  act_err = act_tvel - vel_from_obd;
  cur_err = tvel - vel_from_obd;

  switch (pedal_state)
  {
  case ACCELERATOR:
    printf("ACCEL\n");
    kp = ecat_var[ECAT_KPT].value;
    ki = ecat_var[ECAT_KIT].value;
    kd = ecat_var[ECAT_KDT].value;
    if (change_pedal)
    {
      if (vel_from_obd < tvel)
      {
        check_vel[check_vel_count] = vel_from_obd;
        ++check_vel_count;
        if (check_vel_count > 3)
        {
          if (check_vel[0] >= check_vel[3])
          {
            I_err = (15000 + 1000 * (check_vel[0] - check_vel[3])) / ecat_var[ECAT_KIT].value;
          }
          change_pedal = 0;
          check_vel_count = 0;
        }
      }
      else
      {
        change_pedal = 0;
        check_vel_count = 0;
      }
    }
    if (cur_err < 0)
    {
      if (motor_info[ACCEL_SLAVE].act_pos == 0 || vel_from_obd > tvel + 3)
      {
        if (maintain1)
        {
          if (act_err < check_err)
          {
            ++check_count;
          }
          else if (act_err > check_err)
          {
            if (!check_count)
            {
              --check_count;
            }
          }
          if (check_count > 3)
          {
            check_count = 0;
            maintain1 = 0;
          }
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
        }
        else
        {
          pedal_state = DECELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPB].value;
          ki = ecat_var[ECAT_KIB].value;
          kd = ecat_var[ECAT_KDB].value;
          check_count = 0;
          maintain1 = 0;
        }
      }
      else
      {
        kp = ecat_var[ECAT_KPT].value;
        ki = ecat_var[ECAT_KIT].value;
        kd = ecat_var[ECAT_KDT].value;
      }
    }
    else
    {
      //maintain = 0;
      maintain1 = 0;
      check_count = 0;
    }
    break;
  case DECELERATOR:
    printf("DECEL\n");
    kp = ecat_var[ECAT_KPB].value;
    ki = ecat_var[ECAT_KIB].value;
    kd = ecat_var[ECAT_KDB].value;
    if (cur_err > 0)
    {
      if (motor_info[BRAKE_SLAVE].act_pos == 0 || vel_from_obd < tvel - 3)
      {
        if (maintain1)
        {
          if (act_err > check_err)
          {
            ++check_count;
          }
          else if (act_err < check_err)
          {
            if (!check_count)
            {
              --check_count;
            }
          }
          if (check_count > 3)
          {
            check_count = 0;
            maintain1 = 0;
          }
        }
        else
        {
          change_pedal = 1;
          pedal_state = ACCELERATOR;
          I_err = 0;
          prev_err = 0;
          kp = ecat_var[ECAT_KPT].value;
          ki = ecat_var[ECAT_KIT].value;
          kd = ecat_var[ECAT_KDT].value;
          check_count = 0;
          maintain1 = 0;
        }
      }
    }
    else
    {
      //maintain = 0;
      maintain1 = 0;
      check_count = 0;
    }
    break;
  }

  Kp_term = kp * cur_err;
  I_err += cur_err * dt;
  Ki_term = ki * I_err;
  D_err = (prev_err - cur_err) / dt;
  Kd_term = kd * D_err;

  power = Kp_term + Ki_term + Kd_term;

  prev_err = cur_err;

  sprintf(buf, "Kp_term : %lf\tKi_term : %lf\tPower : %lf\tnACC : %d\tnDEC : %d\n", Kp_term, Ki_term, power, acc_count, dec_count);
  sprintf(buf, "Pedal_state : %d\tMaintain : %d\tCheck_count : %d\tPower : %lf\tAct_tvel : %d\tTvel : %d\tAct_Vel : %d\n", pedal_state, maintain, check_count, power, act_tvel, tvel, vel_from_obd);
  write(pid_fd, buf, strlen(buf));
  //slope_measure(pedal_state, vel_from_obd); // said not using?
  //return;
}

bool PID_CALC2ACCEL::check(HybridAutomata *HA)
{
  if (HA->curState == PID_CALC)
  {
    if (pedal_state == ACCELERATOR)
    {
      cout << "condition : PID_CALC2ACCEL" << endl;
      return true;
    }
  }
  return false;
}
bool PID_CALC2BRAKE::check(HybridAutomata *HA)
{
  if (HA->curState == PID_CALC)
  {
    if (pedal_state == DECELERATOR)
    {
      cout << "condition : PID_CALC2BRAKE" << endl;
      return true;
    }
  }
  return false;
}
void pid_accel()
{
  //cout<<"in pid_accel" <<endl;
  //cout<<"in pid_accel, ecat_var.vel : "<<ecat_var[ECAT_TVEL].value<<endl;
  ++acc_count;
  motor_info[ACCEL_SLAVE].target_pos = power;
  motor_info[BRAKE_SLAVE].target_pos = 0;
  if (motor_info[ACCEL_SLAVE].target_pos > 50000)
  {
    motor_info[ACCEL_SLAVE].target_pos = 50000;
  }
  else if (motor_info[ACCEL_SLAVE].target_pos < 0)
  {
    motor_info[ACCEL_SLAVE].target_pos = 0;
  }
  cout << "[in pid_accel] power : " << motor_info[ACCEL_SLAVE].target_pos << endl;
}
void pid_brake()
{
  //cout<<"in pid_brake" <<endl;
  ++dec_count;
  motor_info[ACCEL_SLAVE].target_pos = 0;
  motor_info[BRAKE_SLAVE].target_pos = -power;
  if (motor_info[BRAKE_SLAVE].target_pos > 230000)
  {
    motor_info[BRAKE_SLAVE].target_pos = 230000;
  }
  else if (motor_info[BRAKE_SLAVE].target_pos < 0)
  {
    motor_info[BRAKE_SLAVE].target_pos = 0;
  }
  cout << "[in pid_brake] power : " << motor_info[BRAKE_SLAVE].target_pos << endl;
}
void pid_finish()
{
  //cout<<"in pid_finish" <<endl;
  return;
}
HybridAutomata *HA_pidcontrol;
void HA_PIDController()
{
  HA_pidcontrol = new HybridAutomata(PID_START, PID_FINISH);
  PID_CALC2ACCEL *pid_calc2accel = new PID_CALC2ACCEL();
  PID_CALC2BRAKE *pid_calc2brake = new PID_CALC2BRAKE();
  HA_pidcontrol->setState(PID_CALC, my_pid_calc2);
  HA_pidcontrol->setState(PID_ACCEL, pid_accel);
  HA_pidcontrol->setState(PID_BRAKE, pid_brake);
  HA_pidcontrol->setState(PID_FINISH, pid_finish);
  HA_pidcontrol->setCondition(PID_START, NULL, PID_CALC);
  HA_pidcontrol->setCondition(PID_CALC, pid_calc2accel, PID_ACCEL);
  HA_pidcontrol->setCondition(PID_CALC, pid_calc2brake, PID_BRAKE);
  HA_pidcontrol->setCondition(PID_ACCEL, NULL, PID_FINISH);
  HA_pidcontrol->setCondition(PID_BRAKE, NULL, PID_FINISH);
  HA_pidcontrol->setCondition(PID_FINISH, NULL, PID_FINISH);
}

/////////////////////////////////////////////////
//All About MOTOR_ERROR STATES
////////////////////////////////////////////////

bool MOTOR_ALL2ERROR::check(HybridAutomata *HA)
{
  for (int index = 0; index < SLAVE_NUM; ++index)
  {
    if (test_flags(MASK_STATUSWORD, FLAGS_ERROR, &motor_info[index].status))
    {
      motor_err_idx = index;
      before_value = HA->curState;
      return true;
    }
  }
  //cout << "condition : MOTOR_ALL2ERROR" << endl;
  return false;
}

bool MOTOR_ERROR2UP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_UP)
    {
      // cout << "condition : MOTOR_ERROR2UP" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2ON::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_ON)
    {
      //  cout << "condition : MOTOR_ERROR2ON" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2READY::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_READY)
    {
      //cout << "condition : MOTOR_ERROR2READY" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2AUTO::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_AUTO)
    {
      //cout << "condition : MOTOR_ERROR2AUTO" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2PID::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_PID)
    {
      //cout << "condition : MOTOR_ERROR2PID" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2RESET::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_RESET)
    {
      //cout << "condition : MOTOR_ERROR2RESET" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2REGULAR_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_REGULAR_STOP)
    {
      //cout << "condition : MOTOR_ERROR2REGULAR_STOP" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_ERROR2EMERGENCY_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_ERROR)
  {
    if (before_value == MOTOR_EMERGENCY_STOP)
    {
      //cout << "condition : MOTOR_ERROR2EMERGENCY_STOP" << endl;
      return true;
    }
  }
  return false;
}
///////////////////////////////////////////////////
//All About Cycle Conditions
//////////////////////////////////////////////////

bool MOTOR_UP2UP::check(HybridAutomata *HA)
{

  if (HA->curState == MOTOR_UP)
  {
    for (int index = 0; index < SLAVE_NUM; ++index)
    {
      if (master_info.slave_state[index].al_state != 0x08)
      {
        return true;
      }
    }
  }
  return false;
}

bool MOTOR_ON2ON::check(HybridAutomata *HA)
{
  bool result = false;
  if (HA->curState == MOTOR_ON)
  {
    for (int index = 0; index < SLAVE_NUM; ++index)
    {
      if (test_flags(MASK_STATUSWORD, FLAGS_SWITCHON_DISABLED, &motor_info[index].status))
      {
        EC_WRITE_U16(master_info.domain_pd + offset_csp[index].ctrl_word, 0x0006);
        result = true;
        //cout << "IN IF index : "<< index << endl;
      }
      else if (test_flags(MASK_STATUSWORD, FLAGS_READYTO_SWITCHON, &motor_info[index].status))
      {
        EC_WRITE_U16(master_info.domain_pd + offset_csp[index].ctrl_word, 0x000F);
        result = true;
        // cout << "IN ELSE IF index : "<< index << endl;
      }
      else //if(test_flags(MASK_STATUSWORD, FLAGS_OPERATION_ENABLE, &motor_info[index].status))
      {
        // cout << "IN ELSE index : "<< index << endl;
        result = false;
      }
    }
  }
  return result;
}
//////////////////////////////////////////////////
//All about State 2 State Conditions
/////////////////////////////////////////////////
bool MOTOR_UP2ON::check(HybridAutomata *HA)
{

  int cnt = 0;
  if (HA->curState == MOTOR_UP)
  {
    for (int index = 0; index < SLAVE_NUM; ++index)
    {
      if (master_info.slave_state[index].al_state == 0x08)
      {
        cnt++;
      }
    }
    if (cnt == SLAVE_NUM)
    {
      cout << "condition : MOTOR_UP2ON" << endl;
      return true;
    }
  }
  //cout << "\tcount : "<<count<<endl;
  //cout << "condition : MOTOR_UP2ON" << endl;
  return false;
}

bool MOTOR_ON2READY::check(HybridAutomata *HA)
{

  if (HA->curState == MOTOR_ON)
  {
    for (int index = 0; index < SLAVE_NUM; ++index)
    {
      if (test_flags(MASK_STATUSWORD, FLAGS_OPERATION_ENABLE, &motor_info[index].status))
      {
        switch (index)
        {
          //case GEAR_SLAVE :
          //sprintf(log_buf, "Gear Slave On\n");
          //write(fd, log_buf, strlen(log_buf));
          //break;
        case HANDLE_SLAVE:
          //sprintf(log_buf, "Handle Slave On\n");
          printf("Handle Slave On\n");
          // write(motor_fd, log_buf, strlen(log_buf));
          break;
        case ACCEL_SLAVE:
          //sprintf(log_buf, "Accel Slave On\n");
          printf("Accel Slave On\n");
          // write(motor_fd, log_buf, strlen(log_buf));
          break;
        case BRAKE_SLAVE:
          //sprintf(log_buf, "Brake Slave On\n");
          printf("Brake Slave On\n");
          // write(motor_fd, log_buf, strlen(log_buf));
          break;
        }
      }
      //else return false;
    }
    cout << "condition : MOTOR_ON2READY" << endl;
    return true;
  }

  return false;
}
bool MOTOR_READY2AUTO::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_READY)
  {
    if (ecat_var[ECAT_MOTOR_STATE].value == MOTOR_AUTO)
    {
      cout << "condition : MOTOR_READY2AUTO" << endl;
      cout << "ecat_var[ECAT_TVEL].value : " << ecat_var[ECAT_TVEL].value << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_READY2RESET::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_READY)
  {
    if (ecat_var[ECAT_MOTOR_STATE].value == MOTOR_RESET)
    {
      cout << "condition : MOTOR_READY2RESET" << endl;

      return true;
    }
  }
  return false;
}
bool MOTOR_AUTO2PID::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_AUTO)
  {
    if ((ecat_var[ECAT_MOTOR_STATE].value != MOTOR_RESET) &&
        (ecat_var[ECAT_MOTOR_STATE].value != MOTOR_REGULAR_STOP) &&
        (ecat_var[ECAT_MOTOR_STATE].value != MOTOR_EMERGENCY_STOP))
    {
      if (glob_vel_timestamp != obd2_var[OBD2_VEL].timestamp)
      {

        glob_vel_timestamp = obd2_var[OBD2_VEL].timestamp;
        cout << "condition : MOTOR_AUTO2PID" << endl;
        cout << "ecat_var[ECAT_TVEL].value : " << ecat_var[ECAT_TVEL].value << endl;
        return true;
      }
    }
  }
  return false;
}
bool MOTOR_AUTO2AUTO::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_AUTO)
  {
    if ((ecat_var[ECAT_MOTOR_STATE].value != MOTOR_RESET) &&
        (ecat_var[ECAT_MOTOR_STATE].value != MOTOR_REGULAR_STOP) &&
        (ecat_var[ECAT_MOTOR_STATE].value != MOTOR_EMERGENCY_STOP))
    {
      if (glob_vel_timestamp == obd2_var[OBD2_VEL].timestamp)
      {
        //cout << "condition : MOTOR_AUTO2AUTO" << endl;
        return true;
      }
    }
  }
  return false;
}

bool MOTOR_READY2READY::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_READY)
  {
    if ((ecat_var[ECAT_MOTOR_STATE].value != MOTOR_AUTO) && (ecat_var[ECAT_MOTOR_STATE].value != MOTOR_RESET))
    {
      return true;
    }
  }
  return false;
}

bool MOTOR_AUTO2RESET::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_AUTO)
  {
    if (ecat_var[ECAT_MOTOR_STATE].value == MOTOR_RESET)
    {
      cout << "condition : MOTOR_AUTO2RESET" << endl;
      return true;
    }
  }
  return false;
}

bool is_motor_reset(int num1, int num2, int num3, int num4)
{
  if (num1)
  {
    if (get_motor_pos(GEAR_SLAVE) > 100)
      return false;
  }
  if (num2)
  {
    if (get_motor_pos(HANDLE_SLAVE) > 100)
      return false;
  }
  if (num3)
  {
    cout << " num3 : " << get_motor_pos(ACCEL_SLAVE) << endl;
    if (get_motor_pos(ACCEL_SLAVE) > 100)
      return false;
  }
  if (num4)
  {
    cout << " num4 : " << get_motor_pos(BRAKE_SLAVE) << endl;
    if (get_motor_pos(BRAKE_SLAVE) > 100)
      return false;
  }
  return true;
}
bool MOTOR_RESET2READY::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_RESET)
  {
    if (is_motor_reset(0, 0, 1, 1) == true)
    {
      cout << "condition : MOTOR_RESET2READY" << endl;
      ecat_var[ECAT_MOTOR_STATE].value = MOTOR_READY;
      return true;
    }
  }
  return false;
}
bool MOTOR_RESET2RESET::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_RESET)
  {
    if (is_motor_reset(0, 0, 1, 1) == false)
    {
      //cout << "condition : MOTOR_RESET2RESET" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_AUTO2REGULAR_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_AUTO)
  {
    if (ecat_var[ECAT_MOTOR_STATE].value == MOTOR_REGULAR_STOP)
    {
      cout << "condition : MOTOR_AUTO2REGULAR_STOP" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_REGULAR_STOP2REGULAR_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_REGULAR_STOP)
  {
    if ((motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE)) < 190000)
    {
      cout << "condition : MOTOR_REGUALR_STOP2REGULAR_STOP" << endl;
      return true;
    }
  }
  return false;
}

bool MOTOR_REGULAR_STOP2READY::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_REGULAR_STOP)
  {
    if ((motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE)) > 190000)
    {
      ecat_var[ECAT_MOTOR_STATE].value = MOTOR_READY;
      count = 0;
      cout << "condition : MOTOR_REGUALR_STOP2READY" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_AUTO2EMERGENCY_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_AUTO)
  {
    if (ecat_var[ECAT_MOTOR_STATE].value == MOTOR_EMERGENCY_STOP)
    {
      cout << "condition : MOTOR_AUTO2EMERGENCY_STOP" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_EMERGENCY_STOP2EMERGENCY_STOP::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_EMERGENCY_STOP)
  {
    if ((motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE)) < 190000)
    {
      cout << "condition : MOTOR_EMERGENCY_STOP2EMERGENCY_STOP" << endl;
      return true;
    }
  }
  return false;
}
bool MOTOR_EMERGENCY_STOP2READY::check(HybridAutomata *HA)
{
  if (HA->curState == MOTOR_EMERGENCY_STOP)
  {
    if ((motor_info[BRAKE_SLAVE].act_pos = get_motor_pos(BRAKE_SLAVE)) > 190000)
    {
      ecat_var[ECAT_MOTOR_STATE].value = MOTOR_READY;
      cout << "condition : MOTOR_EMERGENCY_STOP2READY" << endl;
      return true;
    }
  }
  return false;
}
//////////////////////////////////////////////////////////////
void postprocessing_ecat()
{
  struct timespec present_time;
  if (clock_gettime(CLOCK_MONOTONIC, &present_time))
  {
    perror("clock_gettime");
    exit(EXIT_FAILURE);
  }
  ecrt_master_application_time(master_info.master, TIMESPEC2NS(present_time));
  ecrt_master_sync_reference_clock(master_info.master);
  ecrt_master_sync_slave_clocks(master_info.master);

  ecrt_domain_queue(master_info.domain);
  ecrt_master_send(master_info.master);
  usleep(1000000 / TASK_FREQUENCY);
}
void preprocessing_ecat()
{
  int index;

  ecrt_master_receive(master_info.master);
  ecrt_domain_process(master_info.domain);

  check_domain_state();
  check_master_state();

  for (index = 0; index < SLAVE_NUM; ++index)
  {
    ecrt_slave_config_state(master_info.sc_epos[index], &master_info.slave_state[index]);
  }

  for (index = 0; index < SLAVE_NUM; ++index)
  {
    motor_info[index].status = EC_READ_U16(master_info.domain_pd + offset_csp[index].status_word);
  }
}
void motor_error()
{
  //cout << "\t\tmotor_error" <<endl;
  ecat_var[ECAT_MOTOR_STATE].value = MOTOR_ERROR;
  EC_WRITE_U16(master_info.domain_pd + offset_csp[motor_err_idx].ctrl_word, 0x000F);
  preprocessing_ecat();
  EC_WRITE_U16(master_info.domain_pd + offset_csp[motor_err_idx].ctrl_word, 0x0080);
}
void motor_up()
{
  //cout << "\t\tmotor_up" <<endl;
  ecat_var[ECAT_MOTOR_STATE].value = MOTOR_UP;
}
void motor_on()
{
  //cout << "\t\tmotor_on" <<endl;
  //cout<< " count : "<<cnt++<<endl;
  ecat_var[ECAT_MOTOR_STATE].value = MOTOR_ON;
}

HybridAutomata *HA_ecatcyclic;
void motor_ready()
{
  for (int index = 0; index < SLAVE_NUM; ++index)
  {
    motor_info[index].act_pos = get_motor_pos(index);
    set_motor_pos(index);
  }
}
void motor_auto()
{
  for (int index = 0; index < SLAVE_NUM; ++index)
  {
    motor_info[index].act_pos = get_motor_pos(index);
    set_motor_pos(index);
  }
}
void motor_pid()
{
  //cout << "\t\tmotor_pid" << endl;

  while (1)
  {
    //HA_pidcontrol->curState=0;
    //printf("#START HA_pidcontrol : %d\n", HA_pidcontrol->curState);
    HA_pidcontrol->operate();
    //printf("#MIDDLE HA_pidcontrol : %d\n", HA_pidcontrol->curState);
    if (HA_pidcontrol->curState == PID_FINISH)
    {
      printf("#END HA_pidcontrol : %d\n", HA_pidcontrol->curState);
      HA_pidcontrol->curState = PID_START;
      break;
    }
  }
}
void reset_motor(int num1, int num2, int num3, int num4)
{
  if (num1)
  {
    motor_info[GEAR_SLAVE].target_pos = 0;
    set_motor_pos(GEAR_SLAVE);
  }
  if (num2)
  {
    motor_info[HANDLE_SLAVE].target_pos = 0;
    set_motor_pos(HANDLE_SLAVE);
  }
  if (num3)
  {
    cout << "num3 : " << motor_info[ACCEL_SLAVE].act_pos << endl;
    motor_info[ACCEL_SLAVE].target_pos = 0;
    set_motor_pos(ACCEL_SLAVE);
  }
  if (num4)
  {
    cout << "num4 : " << motor_info[BRAKE_SLAVE].act_pos << endl;
    motor_info[BRAKE_SLAVE].target_pos = 0;
    set_motor_pos(BRAKE_SLAVE);
  }
}
void motor_reset()
{
  cout << "\t\tmotor_reset" << endl;
  ecat_var[ECAT_TVEL].value = 0;
  reset_motor(0, 0, 1, 1);
}
void motor_regular_stop()
{
  cout << "\t\tmotor_regular_stop" << endl;
  cout << " act_pos: " << motor_info[BRAKE_SLAVE].act_pos << endl;
  cout << " target_pos: " << motor_info[BRAKE_SLAVE].target_pos << endl;
  cout << " count: " << count << endl;
  motor_info[ACCEL_SLAVE].target_pos = 0;
  set_motor_pos(ACCEL_SLAVE);
  motor_info[BRAKE_SLAVE].target_pos = brake_start_pos + brake_pos_p_tick * count;
  set_motor_pos(BRAKE_SLAVE);
  ++count;
}
void motor_emergency_stop()
{
  cout << "\t\tmotor_emergency_stop" << endl;
  cout << " act_pos: " << motor_info[BRAKE_SLAVE].act_pos << endl;
  cout << " target_pos: " << motor_info[BRAKE_SLAVE].target_pos << endl;
  motor_info[ACCEL_SLAVE].target_pos = 0;
  motor_info[BRAKE_SLAVE].target_pos = BRAKE_MAX_POS;
  set_motor_pos(BRAKE_SLAVE);
}

void HA_EcatCyclicThread()
{
  cout << "in HA_EcatCyclicThread func()" << endl;

  HA_ecatcyclic = new HybridAutomata(MOTOR_START, MOTOR_FINISH);
  HA_ecatcyclic->setState(MOTOR_ERROR, motor_error);
  HA_ecatcyclic->setState(MOTOR_UP, motor_up);
  HA_ecatcyclic->setState(MOTOR_ON, motor_on);
  HA_ecatcyclic->setState(MOTOR_READY, motor_ready);
  HA_ecatcyclic->setState(MOTOR_AUTO, motor_auto);
  HA_ecatcyclic->setState(MOTOR_PID, motor_pid);
  HA_ecatcyclic->setState(MOTOR_RESET, motor_reset);
  HA_ecatcyclic->setState(MOTOR_REGULAR_STOP, motor_regular_stop);
  HA_ecatcyclic->setState(MOTOR_EMERGENCY_STOP, motor_emergency_stop);

  //////////////////////setting conditions * 2 motor_error//////////////
  MOTOR_ALL2ERROR *motor_all2err = new MOTOR_ALL2ERROR();
  HA_ecatcyclic->setCondition(MOTOR_UP, motor_all2err, MOTOR_ERROR);
  HA_ecatcyclic->setCondition(MOTOR_ON, motor_all2err, MOTOR_ERROR);
  HA_ecatcyclic->setCondition(MOTOR_READY, motor_all2err, MOTOR_ERROR);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_all2err, MOTOR_ERROR);
  //HA_ecatcyclic->setCondition(MOTOR_PID, motor_all2err,MOTOR_ERROR);
  //HA_ecatcyclic->setCondition(MOTOR_RESET, motor_all2err,MOTOR_ERROR);
  //HA_ecatcyclic->setCondition(MOTOR_REGULAR_STOP, motor_all2err,MOTOR_ERROR);
  //HA_ecatcyclic->setCondition(MOTOR_EMERGENCY_STOP, motor_all2err,MOTOR_ERROR);

  //////////////////////setting conditions motor_error 2 *//////////////
  MOTOR_ERROR2UP *motor_error2up = new MOTOR_ERROR2UP();
  MOTOR_ERROR2ON *motor_error2on = new MOTOR_ERROR2ON();
  MOTOR_ERROR2READY *motor_error2ready = new MOTOR_ERROR2READY();
  MOTOR_ERROR2AUTO *motor_error2auto = new MOTOR_ERROR2AUTO();
  MOTOR_ERROR2PID *motor_error2pid = new MOTOR_ERROR2PID();
  MOTOR_ERROR2RESET *motor_error2reset = new MOTOR_ERROR2RESET();
  MOTOR_ERROR2REGULAR_STOP *motor_error2regular_stop = new MOTOR_ERROR2REGULAR_STOP();
  MOTOR_ERROR2EMERGENCY_STOP *motor_error2emergency_stop = new MOTOR_ERROR2EMERGENCY_STOP();
  HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2up, MOTOR_UP);
  HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2on, MOTOR_ON);
  HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2ready, MOTOR_READY);
  HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2auto, MOTOR_AUTO);
  //HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2pid,MOTOR_PID);
  //HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2reset,MOTOR_RESET);
  //HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2regular_stop,MOTOR_REGULAR_STOP);
  //HA_ecatcyclic->setCondition(MOTOR_ERROR, motor_error2emergency_stop,MOTOR_EMERGENCY_STOP);

  //////////////////////setting conditions of cycle state//////////////
  MOTOR_UP2UP *motor_up2up = new MOTOR_UP2UP();
  MOTOR_ON2ON *motor_on2on = new MOTOR_ON2ON();
  MOTOR_READY2READY *motor_ready2ready = new MOTOR_READY2READY();
  MOTOR_AUTO2AUTO *motor_auto2auto = new MOTOR_AUTO2AUTO();
  MOTOR_RESET2RESET *motor_reset2reset = new MOTOR_RESET2RESET();
  MOTOR_REGULAR_STOP2REGULAR_STOP *motor_regular_stop2regular_stop = new MOTOR_REGULAR_STOP2REGULAR_STOP();
  MOTOR_EMERGENCY_STOP2EMERGENCY_STOP *motor_emergency_stop2emergency_stop = new MOTOR_EMERGENCY_STOP2EMERGENCY_STOP();

  HA_ecatcyclic->setCondition(MOTOR_UP, motor_up2up, MOTOR_UP);
  HA_ecatcyclic->setCondition(MOTOR_ON, motor_on2on, MOTOR_ON);
  HA_ecatcyclic->setCondition(MOTOR_READY, motor_ready2ready, MOTOR_READY);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_auto2auto, MOTOR_AUTO);
  HA_ecatcyclic->setCondition(MOTOR_RESET, motor_reset2reset, MOTOR_RESET);
  HA_ecatcyclic->setCondition(MOTOR_REGULAR_STOP, motor_regular_stop2regular_stop, MOTOR_REGULAR_STOP);
  HA_ecatcyclic->setCondition(MOTOR_EMERGENCY_STOP, motor_emergency_stop2emergency_stop, MOTOR_EMERGENCY_STOP);

  //////////////////////setting conditions of state 2 state//////////////

  MOTOR_UP2ON *motor_up2on = new MOTOR_UP2ON();
  MOTOR_ON2READY *motor_on2ready = new MOTOR_ON2READY();
  MOTOR_READY2AUTO *motor_ready2auto = new MOTOR_READY2AUTO();
  MOTOR_READY2RESET *motor_ready2reset = new MOTOR_READY2RESET();
  MOTOR_AUTO2PID *motor_auto2pid = new MOTOR_AUTO2PID();
  MOTOR_AUTO2RESET *motor_auto2reset = new MOTOR_AUTO2RESET();
  MOTOR_AUTO2REGULAR_STOP *motor_auto2regular_stop = new MOTOR_AUTO2REGULAR_STOP();
  MOTOR_AUTO2EMERGENCY_STOP *motor_auto2emergency_stop = new MOTOR_AUTO2EMERGENCY_STOP();
  MOTOR_RESET2READY *motor_reset2ready = new MOTOR_RESET2READY();
  MOTOR_REGULAR_STOP2READY *motor_regular_stop2ready = new MOTOR_REGULAR_STOP2READY();
  MOTOR_EMERGENCY_STOP2READY *motor_emergency_stop2ready = new MOTOR_EMERGENCY_STOP2READY();

  HA_ecatcyclic->setCondition(MOTOR_START, NULL, MOTOR_UP);
  HA_ecatcyclic->setCondition(MOTOR_UP, motor_up2on, MOTOR_ON);
  HA_ecatcyclic->setCondition(MOTOR_ON, motor_on2ready, MOTOR_READY);
  HA_ecatcyclic->setCondition(MOTOR_READY, motor_ready2auto, MOTOR_AUTO);
  HA_ecatcyclic->setCondition(MOTOR_READY, motor_ready2reset, MOTOR_RESET);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_auto2pid, MOTOR_PID);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_auto2reset, MOTOR_RESET);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_auto2emergency_stop, MOTOR_EMERGENCY_STOP);
  HA_ecatcyclic->setCondition(MOTOR_AUTO, motor_auto2regular_stop, MOTOR_REGULAR_STOP);
  HA_ecatcyclic->setCondition(MOTOR_RESET, motor_reset2ready, MOTOR_READY);
  HA_ecatcyclic->setCondition(MOTOR_REGULAR_STOP, motor_regular_stop2ready, MOTOR_READY);
  HA_ecatcyclic->setCondition(MOTOR_PID, NULL, MOTOR_AUTO);
  HA_ecatcyclic->setCondition(MOTOR_EMERGENCY_STOP, motor_emergency_stop2ready, MOTOR_READY);
}
void *cyclic_task(void *m)
{
  char log_buf[1024];

  struct sched_param sparam;
  sparam.sched_priority = 49;
  sched_setscheduler(0, SCHED_FIFO, &sparam);

  motor_fd = open("motor_log.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
  pid_fd = open("pid_log.txt", O_CREAT | O_RDWR | O_APPEND, 0666);

  if (motor_fd < 0)
  {
    printf("Motor Log Err\n");
    return NULL;
  }
  if (pid_fd < 0)
  {
    printf("Pid Log Err\n");
    return NULL;
  }

  init_motor();
  HA_PIDController();
  while (1)
  {
    preprocessing_ecat();
    //cout << "curState : "<< HA_ecatcyclic->curState<<endl;
    //cout<< "ecat_var[ECAT_MOTOR_STATE].value :" << ecat_var[ECAT_MOTOR_STATE].value << endl;
    HA_ecatcyclic->operate();

    postprocessing_ecat();

    sprintf(log_buf, "GEAR : %d\tHANDLE : %d\tACCEL : %d\tBRAKE : %d\tVEL : %d\t\n", motor_info[GEAR_SLAVE].act_pos, motor_info[HANDLE_SLAVE].act_pos, motor_info[ACCEL_SLAVE].act_pos, motor_info[BRAKE_SLAVE].act_pos, obd2_var[OBD2_VEL].value);
    write(motor_fd, log_buf, strlen(log_buf));
  } //end of while

  return NULL;
}

void ecat_up()
{
  int index;

  master_info.master = ecrt_request_master(0);
  if (!master_info.master)
  {
    exit(EXIT_FAILURE);
  }

  master_info.domain = ecrt_master_create_domain(master_info.master);
  if (!master_info.domain)
  {
    exit(EXIT_FAILURE);
  }

  for (index = 0; index < SLAVE_NUM; ++index)
  {
    if (!(master_info.sc_epos[index] = ecrt_master_slave_config(master_info.master, 0, index, MAXON_EPOS)))
    {
      fprintf(stderr, "Failed to get slave configuration for EPOS\n");
      exit(EXIT_FAILURE);
    }
  }

  printf("config sdo\n");
  for (index = 0; index < SLAVE_NUM; ++index)
  {
    ecrt_slave_config_sdo8(master_info.sc_epos[index], 0x6060, 0x00, 0x08);
    ecrt_slave_config_sdo32(master_info.sc_epos[index], 0x6065, 0x00, 0xFFFFFFFF);
  }

#if CONFIGURE_PDOS
  printf("Configuring PDO\n");

  for (index = 0; index < SLAVE_NUM; ++index)
  {
    if (ecrt_slave_config_pdo(master_info.sc_epos[index], EC_END, EPOS_pdo_1_syncs))
    {
      fprintf(stderr, "Failed to configure PODs\n");
      exit(EXIT_FAILURE);
    }
  }
  printf("Configuring PDO is completed!\n");
#endif

  if (ecrt_domain_reg_pdo_entry_list(master_info.domain, domain1_regs1))
  {
    fprintf(stderr, "PDO entry registration failed\n");
  }
  if (ecrt_domain_reg_pdo_entry_list(master_info.domain, domain1_regs2))
  {
    fprintf(stderr, "PDO entry registration failed\n");
  }
  if (ecrt_domain_reg_pdo_entry_list(master_info.domain, domain1_regs3))
  {
    fprintf(stderr, "PDO entry registration failed\n");
  }
  if (ecrt_domain_reg_pdo_entry_list(master_info.domain, domain1_regs4))
  {
    fprintf(stderr, "PDO entry registration failed\n");
  }

  for (index = 0; index < SLAVE_NUM; ++index)
  {
    ecrt_slave_config_dc(master_info.sc_epos[index], 0x0300, 1000000, 440000, 0, 0);
  }

  printf("Activating master..\n");

  if (ecrt_master_activate(master_info.master))
  {
    exit(EXIT_FAILURE);
  }

  if (!(master_info.domain_pd = ecrt_domain_data(master_info.domain)))
  {
    exit(EXIT_FAILURE);
  }

  master_info.ecat_state = ECAT_UP;
}

void ecat_on()
{
  master_info.ecat_state = ECAT_ON;
  if (pthread_create(&master_info.cyclic_thread, 0, cyclic_task, NULL))
  {
    printf("Thread Err\n");
    exit(EXIT_FAILURE);
  }
}

void ecat_down()
{
  master_info.ecat_state = ECAT_DOWN;
  cout << "ecat_down called" << endl;
  ecrt_master_deactivate(master_info.master);
}
void ecat_off()
{
  master_info.ecat_state = ECAT_OFF;
  cout << "ecat_off called" << endl;
  pthread_cancel(master_info.cyclic_thread);
  pthread_join(master_info.cyclic_thread, NULL);
}

void get_slave_state(int index)
{
  ecrt_slave_config_state(master_info.sc_epos[index], &master_info.slave_state[index]);
  return;
}

void check_domain_state()
{
  ec_domain_state_t ds;

  ecrt_domain_state(master_info.domain, &ds);

  if (ds.working_counter != master_info.domain_state.working_counter)
    printf("Domain1: WC %u.\n", ds.working_counter);
  if (ds.wc_state != master_info.domain_state.wc_state)
    printf("Domain1: State %u.\n", ds.wc_state);

  master_info.domain_state = ds;
  return;
}

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
  struct timespec result;

  if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC)
  {
    result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
    result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
  }
  else
  {
    result.tv_sec = time1.tv_sec + time2.tv_sec;
    result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
  }

  return result;
}

void check_master_state()
{
  ec_master_state_t ms;

  ecrt_master_state(master_info.master, &ms);

  if (ms.slaves_responding != master_info.master_state.slaves_responding)
    printf("%u slave(s).\n", ms.slaves_responding);
  if (ms.al_states != master_info.master_state.al_states)
    printf("AL states: 0x%02X.\n", ms.al_states);
  if (ms.link_up != master_info.master_state.link_up)
    printf("Link is %s.\n", ms.link_up ? "up" : "down");

  master_info.master_state = ms;
}

#if SDO_ACCESS
void read_sdo(void)
{
  switch (ecrt_sdo_request_state(sdo))
  {
  case EC_REQUEST_UNUSED:
    ecrt_sdo_request_read(sdo);
    break;
  case EC_REQUEST_BUSY:
    fprintf(stderr, "Still busy\n");
    break;
  case EC_REQUEST_SUCCESS:
    fprintf(stderr, "SDO value : 0x%04X\n", EC_READ_U16(ecrt_sdo_request_data(sdo)));
    ecrt_sdo_request_read(sdo);
    break;
  case EC_REQUEST_ERROR:
    fprintf(stderr, "Failed to read SDO\n");
    ecrt_sdo_request_read(sdo);
    break;
  }
}

void write_sdo(ec_sdo_request_t *sdo, uint8_t *data, size_t size)
{
  switch (ecrt_sdo_request_state(sdo))
  {
  case EC_REQUEST_BUSY:
    ecrt_sdo_request_write(sdo);
    break;
  case EC_REQUEST_UNUSED:
  case EC_REQUEST_SUCCESS:
    if (size == 8)
      EC_WRITE_U64(ecrt_sdo_request_data(sdo), *((uint64_t *)data));
    else if (size == 4)
      EC_WRITE_U32(ecrt_sdo_request_data(sdo), *((uint32_t *)data));
    else if (size == 2)
      EC_WRITE_U16(ecrt_sdo_request_data(sdo), *((uint16_t *)data));
    else
      EC_WRITE_U8(ecrt_sdo_request_data(sdo), *((uint8_t *)data));
    ecrt_sdo_request_write(sdo);
  case EC_REUQEST_ERROR:
    fprintf(stderr, "Failed to write SDO! data:[0x%X], size:[%d]\n", *data, size);
    ecrt_sdo_request_write(sdo);
    break;
  }
}
extern Var obd2_var[OBD2_MAX];
extern Var ecat_var[ECAT_MAX];
#endif
