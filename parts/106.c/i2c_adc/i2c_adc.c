
Adafruit_ADS1115 i2c_adc_;

bool i2c_adc_init_done_ = false;

// address must be 0x49 (default is 0x48)
#define i2c_adc_address 0x49

void i2c_adc(const char * args)
{
  if (!i2c_adc_init_done_) {
    if (!i2c_adc_.begin(i2c_adc_address)) {
      LOG("'i2c_adc_.begin()' failure");
      return;
    } else {
      LOG("'i2c_adc_.begin()' success");
    }
    i2c_adc_init_done_ = true;
  }

  for (unsigned i = 0; i < 4; ++i) {
    const auto adc = i2c_adc_.readADC_SingleEnded(i);
    const auto volts = i2c_adc_.computeVolts(adc);
    LOG("i2c_adc[%u]=%i/%f", i, adc, volts);
  }
}
