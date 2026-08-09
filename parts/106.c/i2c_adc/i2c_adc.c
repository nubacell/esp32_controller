
//TwoWire twoWire_;
Adafruit_ADS1115 i2c_adc_;

bool twoWire_init_done_ = false;
bool i2c_adc_init_done_ = false;

// address must be 0x49 (default is 0x48)
#define i2c_adc_address 0x49

// 100kHz is actually 94.340kHz
//#define i2c_adc_frequency 100000
//#define i2c_adc_frequency 200000
// 400kHz is actually 384kHz
#define i2c_adc_frequency 400000
//#define i2c_adc_frequency 500000
//#define i2c_adc_frequency 600000
//#define i2c_adc_frequency 650000
// at 675kHz i2c adc frequently fails
//#define i2c_adc_frequency 675000
//#define i2c_adc_frequency 1000000

void i2c_adc(const char * args)
{
  if (!twoWire_init_done_) {
    if (!Wire.begin(-1, -1, i2c_adc_frequency)) {
      LOG("'%s.begin()' %s", "Wire", "failure");
      return;
    } else {
      LOG("'%s.begin()' %s", "Wire", "success");
      twoWire_init_done_ = true;
    }
  }

  if (!i2c_adc_init_done_) {
    //if (!i2c_adc_.begin(i2c_adc_address, &twoWire_)) {
    if (!i2c_adc_.begin(i2c_adc_address, &Wire)) {
      LOG("'%s.begin()' %s", "i2c_adc_", "failure");
      return;
    } else {
      LOG("'%s.begin()' %s", "i2c_adc_", "success");
      i2c_adc_init_done_ = true;
      i2c_adc_.setDataRate(RATE_ADS1115_860SPS);
    }
  }

  int16_t adc[4];
  uint64_t us[5];
  us[0] = esp_timer_get_time();
  for (unsigned i = 0; i < 4; ++i) {
    adc[i] = i2c_adc_.readADC_SingleEnded(i);
    us[i+1] = esp_timer_get_time();
  }
  for (unsigned i = 0; i < 4; ++i) {
    const auto volts = i2c_adc_.computeVolts(adc[i]);
    LOG("us[%u]=%" PRIu64 " - us[%u]=%" PRIu64 " =%" PRIu64 " i2c_adc[%u]=%i/%f",
        i+1, us[i+1], i, us[i], us[i+1] - us[i],
        i, adc[i], volts);
  }
}
