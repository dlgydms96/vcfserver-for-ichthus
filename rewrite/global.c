
int ssegNo = 0;
int rsegNo = 0;

int motor_fsm_state = 1;

Var app_var[] = {
  {"help", 0, 1001, 0, 0, 0, 0},
  {"quit", 0, 1002, 0, 0, 0, 0},
  //  {"reset", 0, 1003, 0, 0, 0, 0},
  {"all", 0, 1004, 0, 0, 0, 0},
};

Var ecat_var[] = {
  {"kpt", 600, 1, 0, 0, 0, 0},
  {"kit", 25, 2, 0, 0, 0, 0},
  {"kdt", 300, 3, 0, 0, 0, 0},
  {"kpb", 3000, 4, 0, 0, 0, 0},
  {"kib", 250, 5, 0, 0, 0, 0},
  {"kdb", 2000, 6, 0, 0, 0, 0},
  {"tvel", 0, 7, 0, 0, 0, 0},
  {"gpos", 0, 8, 0, 0, 0, 0},
  {"spos", 0, 9, 0, 0, 0, 0},
  {"tpos", 0, 10, 0, 0, 0, 0},
  {"bpos", 0, 11, 0, 0, 0, 0},
  {"wsize", 50, 12, 0, 0, 0, 0},
  {"caching", 0, 13, 0, 0, 0, 0},
  {"I_size", 2, 14, 0, 0, 0, 0},
  {"start", 0, 15, 0, 0, 0, 0},
  {"reset", 0, 16, 0, 0, 0, 0},
  {"cach_f", 9, 17, 0, 0, 0, 0},
  {"cach_b", 10, 18, 0, 0, 0, 0},
  {"using_I_err_window", 1, 19, 0, 0, 0, 0},
  {"pull_over", 0, 20, 0, 0, 0, 0},
  {"emergency_stop", 0, 21, 0, 0, 0, 0},
  {"cycle", 5, 22, 0, 0, 0, 0},
  {"stair", 3, 23, 0, 0, 0, 0},
  {"vel_diff", 3, 24, 0, 0, 0},
  {"pidv", 2, 25, 0, 0, 0},
  {"gearstate", 0, 26, 0, 0, 0},
  {"ecatstate", 3, 27, 0, 0, 0, 0},
};

Var obd2_var[] = {
  {"vel", 0, 101, 0, 0, 0, 0},
  {"rpm", 0, 102, 0, 0, 0, 0},
  {"batt", 0, 103, 0, 0, 0, 0},
  {"fuel", 0, 104, 0, 0, 0, 0},
  {"gmode", 0, 105, 0, 0, 0, 0},
  {"airp1", 0, 106, 0, 0, 0, 0},
  {"airp2", 0, 107, 0, 0, 0, 0},
  {"airp3", 0, 108, 0, 0, 0, 0},
  {"airp4", 0, 109, 0, 0, 0, 0},
  {"obdstate", 3, 110, 0, 0, 0, 0},
};

int napp_var = sizeof(app_var) / sizeof(app_var[0]);
int necat_var = sizeof(ecat_var) / sizeof(ecat_var[0]);
int nobd2_var = sizeof(obd2_var) / sizeof(obd2_var[0]);

