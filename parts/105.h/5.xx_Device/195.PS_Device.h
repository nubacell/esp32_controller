
struct PS_Device
  : XX_Device
{
  PS_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : XX_Device(uartConfig_, expected_id_)
  {}

  void clear_alerts() const;

  void ovp() const;
  void ocp() const;
  void output() const;

  void x(const char * s, double v) const;

  void ovp(double v) const;
  void ocp(double v) const;

  void on();
  void off();

  void v() const;
  void v(double v) const;
  void c() const;
  void c(double v) const;
};

extern PS_Device psDevice;
