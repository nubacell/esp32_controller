
struct PS_Device
  : PxDevice
{
  PS_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : PxDevice(uartConfig_, expected_id_)
  {}

  void v() const;
  void c() const;
  void ovp() const;
  void ocp() const;
  void output() const;

  void x(const char * s, double v) const;

  void v(double v) const;
  void c(double v) const;
  void ovp(double v) const;
  void ocp(double v) const;
  void output_ON() const;
  void output_OFF() const;
};

extern PS_Device psDevice;
