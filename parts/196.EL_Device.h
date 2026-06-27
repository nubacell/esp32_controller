
struct EL_Device
  : PxDevice
{
  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}

  // 30 2e 31 30 30 30 41 0a                           |0.1000A.|
  // u2_lf :CURRent?
  void get_I()
  {
    write_cmd(":CURRent?");
  }

  // 30 2e 31 30 30 30 56 0a                           |0.1000V.|
  // u2_lf :VOLTage?
  void get_V()
  {
    write_cmd(":VOLTage?");
  }

  // u2_lf :CURRent?
  // u2_lf :CURRent 1A
  // u2_lf :CURRent 30A
  // u2_lf :CURRent 12.345A
  void set_I(double v)
  {
    char cmd[sizeof(":CURRent1 9.9999A")];
    snprintf(cmd, sizeof(cmd), ":CURRent1 %.4fA", v);
    write_cmd(cmd);

    get_I();
  }

  // u2_lf :VOLTage 1V
  // u2_lf :VOLTage 120V
  // u2_lf :VOLTage 1000V
  // u2_lf :VOLTage 113.45V
  // u2_lf :VOLTage 111.22V
  // u2_lf :VOLTage 12.321V
  void set_V(double v)
  {
    char cmd[sizeof(":VOLTage1:9.9999V")];
    snprintf(cmd, sizeof(cmd), ":VOLTage1 %.4fV", v);
    write_cmd(cmd);

    get_V();
  }
};

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
