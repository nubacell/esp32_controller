
struct EL_Device
  : PxDevice
{
  uint8_t function_;
  uint8_t cc_range_;
  uint8_t cv_range_;

  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}

  void function() const
  {
    const char * cmd = "INPut:FUNCtion?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void function_xx(unsigned v)
  {
    char cmd[sizeof("INPut:FUNCtion XX")];
    snprintf(cmd, sizeof(cmd), "INPut:FUNCtion %s", el_function_to_string(v));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    function_ = 0;
  }

  void function_CC()
  {
    function_xx(0);
  }

  void function_CV()
  {
    function_xx(1);
  }

  /*
    Constant Current

    Example: Set the load to the constant current function, and the main current
    value to 50A, and the programming command is as follows:

    INPut:FUNCtion CC                       // Switch to the constant current test function
    STATic:CC:RANGe 0                       // Set the constant current range to the high range
    STATic:CC:HIGH:LEVel 50.0               // Set the load current to 50.0A
    STATic:CC:HIGH:SLEWRate:RAIse 600       // Set the rise slew to 600.0A/ms
    STATic:CC:HIGH:SLEWRate:FALL 600        // Set the fall slew to 600.0A/ms
    INPut:STATe ON                          // Turn on the load input
  */
  // 0-high range, 1-low range, 2-medium range
  void cc_range() const
  {
    const char * cmd = "STATic:CC:RANGe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_range(unsigned v)
  {
    char cmd[sizeof("STATic:CC:RANGe X")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:RANGe %u", v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    cc_range_ = v;
  }

  void cc_xx_level(unsigned range) const
  {
    char cmd[sizeof("STATic:CC:xxxx:LEVel?")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel?", el_range_to_string(range));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_level() const
  {
    cc_xx_level(cc_range_);
  }

  /*
    u2_lf STATic:CC:HIGH:LEVel 1.12345 OK
    u2_lf STATic:CC:HIGH:LEVel 12.1234 OK
    u2_lf STATic:CC:HIGH:LEVel 123.123 OK
    u2_lf STATic:CC:HIGH:LEVel 1234.12 NG
  */
  void cc_xx_level(unsigned range, double v) const
  {
    char cmd[sizeof("STATic:CC:xxxx:LEVel 123.123")];
    snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel %.5f", el_range_to_string(range), v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cc_level(double v) const
  {
    cc_xx_level(cc_range_, v);
  }

  /*
    Constant Voltage

    Example: Set the load to the constant voltage function, and the main value of
    the constant voltage to 30V. The programming command is as follows:

    INPut:FUNCtion CV             // Switch to the constant voltage test function
    STATic:CV:RANGe 0             // Set the constant current range to the high range
    STATic:CV:RATe 0              // Set the constant voltage response speed
    STATic:CV:HIGH:LEVel 50.0     // Set the load voltage to 50.0V
    INPut:STATe ON                // Turn on the load input
  */
  // 0-high range, 1-low range, 2-medium range
  void cv_range() const
  {
    const char * cmd = "STATic:CV:RANGe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_range(unsigned v)
  {
    char cmd[sizeof("STATic:CV:RANGe X")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:RANGe %u", v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
    cv_range_ = v;
  }

  void cv_xx_level(unsigned range) const
  {
    char cmd[sizeof("STATic:CV:xxxx:LEVel?")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel?", el_range_to_string(range));
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_level() const
  {
    cv_xx_level(cv_range_);
  }

/*
    u2_lf STATic:CV:HIGH:LEVel?
    u2_lf STATic:CV:HIGH:LEVel 1.13245 OK
    u2_lf STATic:CV:HIGH:LEVel 12.1324 OK
    u2_lf STATic:CV:HIGH:LEVel 123.123 OK
    u2_lf STATic:CV:HIGH:LEVel 1234.13 NG
*/
  void cv_xx_level(unsigned range, double v) const
  {
    char cmd[sizeof("STATic:CV:xxxx:LEVel 123.123")];
    snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel %.5f", el_range_to_string(range), v);
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void cv_level(double v) const
  {
    cv_xx_level(cv_range_, v);
  }

  void get_input() const
  {
    const char * cmd = "INPut:STATe?";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void set_input_ON() const
  {
    const char * cmd = "INPut:STATe ON";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void set_input_OFF() const
  {
    const char * cmd = "INPut:STATe OFF";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

#if 0
  // SYST:LOC // Exit remote control mode and return to local control mode.
  void release_control() const
  {
    //const char * cmd = "SYST:LOC";
    const char * cmd = "SYSTem:LOCal";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }
#endif

  void clear_errors() const
  {
    const char * cmd = "*CLS";
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }

  void id() const
  {
    const char * cmd = CMD_IDENTIFY;
    LOG("cmd='%s'", cmd);
    write_cmd(cmd);
  }
};

extern EL_Device elDevice;
