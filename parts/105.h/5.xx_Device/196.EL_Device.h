
struct EL_Device
  : XX_Device
{
  uint8_t function_;
  uint8_t cc_range_;
  uint8_t cv_range_;

  EL_Device(
      const UartConfig * uartConfig_,
      const char * const expected_id_)
    : XX_Device(uartConfig_, expected_id_)
  {}

  void clear_errors() const;

  void function() const;
  void function_xx(unsigned v);
  void function_CC();
  void function_CV();

  // 0-high range, 1-low range, 2-medium range
  void cc_range() const;
  void cc_range(unsigned v);
  void cc_xx_level(unsigned range) const;
  void cc_level() const;
  void cc_xx_level(unsigned range, double v) const;
  void cc_level(double v) const;

  // 0-high range, 1-low range, 2-medium range
  void cv_range() const;
  void cv_range(unsigned v);
  void cv_xx_level(unsigned range) const;
  void cv_level() const;
  void cv_xx_level(unsigned range, double v) const;
  void cv_level(double v) const;
  void get_input() const;

  void on();
  void off();

  void c() const;
  void c(double v);
};

extern EL_Device elDevice;
