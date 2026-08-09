
//TwoWire twoWire_;
Adafruit_ADS1115 i2c_adc_;

bool twoWire_init_done_ = false;
bool i2c_adc_init_done_ = false;

// address must be 0x49 (default is 0x48)
#define i2c_adc_address 0x49

//#define i2c_adc_frequency 100000
//#define i2c_adc_frequency 200000
//#define i2c_adc_frequency 400000
//#define i2c_adc_frequency 500000
//#define i2c_adc_frequency 600000
#define i2c_adc_frequency 650000
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
    }
  }

  for (unsigned i = 0; i < 4; ++i) {
    const auto adc = i2c_adc_.readADC_SingleEnded(i);
    const auto volts = i2c_adc_.computeVolts(adc);
    LOG("i2c_adc[%u]=%i/%f", i, adc, volts);
  }
}
