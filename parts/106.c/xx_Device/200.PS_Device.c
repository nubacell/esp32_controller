
void PS_Device::v() const
{
  const char * cmd = "V1?";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::c() const
{
  const char * cmd = "I1?";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::ovp() const
{
  const char * cmd = "OVP1?";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::ocp() const
{
  const char * cmd = "OCP1?";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::output() const
{
  const char * cmd = "OP1?";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

/*
  "I1"
  "V1"
  "OCP1"
  "OVP1"
*/
void PS_Device::x(const char * s, double v) const
{
  char cmd[sizeof("XXX1 12.123")];
  snprintf(cmd, sizeof(cmd), "%s %.3f", s, v);
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::v(double v) const
{
  x("V1", v);
}

void PS_Device::c(double v) const
{
  x("I1", v);
}

void PS_Device::ovp(double v) const
{
  x("OVP1", v);
}

void PS_Device::ocp(double v) const
{
  x("OCP1", v);
}

void PS_Device::output_ON() const
{
  const char * cmd = "OP1 1";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}

void PS_Device::output_OFF() const
{
  const char * cmd = "OP1 0";
  LOG("cmd='%s'", cmd);
  write_cmd(cmd);
}
