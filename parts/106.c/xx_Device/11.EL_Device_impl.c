
void EL_Device::clear_errors() const
{
  const char * cmd = "*CLS";
  write_cmd(cmd);
}

void EL_Device::function() const
{
  const char * cmd = "INPut:FUNCtion?";
  write_cmd(cmd);
}

void EL_Device::function_xx(unsigned v)
{
  function_ = v;

  char cmd[sizeof("INPut:FUNCtion XX")];
  snprintf(cmd, sizeof(cmd), "INPut:FUNCtion %s", el_function_to_string(v));
  write_cmd(cmd);

  if (cmd_verify_) { function(); }
}

void EL_Device::function_CC()
{
  function_xx(0);
}

void EL_Device::function_CV()
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
void EL_Device::cc_range() const
{
  const char * cmd = "STATic:CC:RANGe?";
  write_cmd(cmd);
}

void EL_Device::cc_range(unsigned v)
{
  cc_range_ = v;

  char cmd[sizeof("STATic:CC:RANGe X")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:RANGe %u", v);
  write_cmd(cmd);

  if (cmd_verify_) { cc_range(); }
}

void EL_Device::cc_xx_level(unsigned range) const
{
  char cmd[sizeof("STATic:CC:xxxx:LEVel?")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel?", el_range_to_string(range));
  write_cmd(cmd);
}

void EL_Device::cc_level() const
{
  cc_xx_level(cc_range_);
}

/*
  u2_lf STATic:CC:HIGH:LEVel 1.12345 OK
  u2_lf STATic:CC:HIGH:LEVel 12.1234 OK
  u2_lf STATic:CC:HIGH:LEVel 123.123 OK
  u2_lf STATic:CC:HIGH:LEVel 1234.12 NG
*/
void EL_Device::cc_xx_level(unsigned range, double v) const
{
  char cmd[sizeof("STATic:CC:xxxx:LEVel 123.123")];
  snprintf(cmd, sizeof(cmd), "STATic:CC:%s:LEVel %.5f", el_range_to_string(range), v);
  write_cmd(cmd);

  if (cmd_verify_) { cc_xx_level(range); }
}

void EL_Device::cc_level(double v) const
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
void EL_Device::cv_range() const
{
  const char * cmd = "STATic:CV:RANGe?";
  write_cmd(cmd);
}

void EL_Device::cv_range(unsigned v)
{
  cv_range_ = v;

  char cmd[sizeof("STATic:CV:RANGe X")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:RANGe %u", v);
  write_cmd(cmd);

  if (cmd_verify_) { cv_range(); }
}

void EL_Device::cv_xx_level(unsigned range) const
{
  char cmd[sizeof("STATic:CV:xxxx:LEVel?")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel?", el_range_to_string(range));
  write_cmd(cmd);
}

void EL_Device::cv_level() const
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
void EL_Device::cv_xx_level(unsigned range, double v) const
{
  char cmd[sizeof("STATic:CV:xxxx:LEVel 123.123")];
  snprintf(cmd, sizeof(cmd), "STATic:CV:%s:LEVel %.5f", el_range_to_string(range), v);
  write_cmd(cmd);

  if (cmd_verify_) { cv_xx_level(range); }
}

void EL_Device::cv_level(double v) const
{
  cv_xx_level(cv_range_, v);
}

void EL_Device::get_input() const
{
  const char * cmd = "INPut:STATe?";
  write_cmd(cmd);
}

void EL_Device::on()
{
  if (on_) { return; }

  const char * cmd = "INPut:STATe ON";
  write_cmd(cmd);

  on_ = true;

  if (cmd_verify_) { get_input(); }
}

void EL_Device::off()
{
  if (!on_) { return; }

  const char * cmd = "INPut:STATe OFF";
  write_cmd(cmd);

  on_ = false;

  if (cmd_verify_) { get_input(); }
}

void EL_Device::c() const
{
  cc_range();
  cc_level();
}

void EL_Device::c(double v)
{
  cc_range(0); // high range
  cc_level(v);
}
