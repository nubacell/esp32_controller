
struct PS_Device
  : PxDevice
{
  PS_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}

  // 31 2e 30 30 30 0a                                 |1.000.|
  // 31 2e 32 33 34 0a                                 |1.234.|
  // 35 2e 31 30 30 0a                                 |5.100.|         ISET1:12.345    ISET1:9.999
  void get_I()
  {
    write_cmd("ISET1?");
  }

  // 30 39 2e 30 30 0a                                 |09.00.|
  // 31 32 2e 33 34 0a                                 |12.34.|
  // 30 30 2e 31 32 0a                                 |00.12.|
  // 33 31 2e 30 30 0a                                 |31.00.|
  // 30 30 2e 30 30 0a                                 |00.00.|
  void get_V()
  {
    write_cmd("VSET1?");
  }

  void set_I(double v)
  {
    char cmd[sizeof("ISET1:9.999")];
    snprintf(cmd, sizeof(cmd), "ISET1:%.3f", v);
    write_cmd(cmd);

    get_I();
  }

  void set_V(double v)
  {
    char cmd[sizeof("VSET1:99.99")];
    snprintf(cmd, sizeof(cmd), "VSET1:%.2f", v);
    write_cmd(cmd);

    get_V();
  }
};

extern PS_Device psDevice;
