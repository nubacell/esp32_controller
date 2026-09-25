
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html

namespace nc {
namespace i2c {

struct Bus
{
  const i2c_master_bus_config_t & i2c_master_bus_config_;
  i2c_master_bus_handle_t i2c_master_bus_handle_;

  bool init_done_ = false;

  bool init();
  void down();

  bool probe(uint16_t address);
};

extern Bus bus;

struct Device
{
  const Bus & bus_;
  const i2c_device_config_t & i2c_device_config_;
  i2c_master_dev_handle_t i2c_master_dev_handle_;

  bool init_done_ = false;

  bool init();
  void down();

  bool read(uint8_t * read_buffer, size_t read_size);
  bool write(const uint8_t * write_buffer, size_t write_size);
  bool write_read(
      const uint8_t * write_buffer, size_t write_size,
      uint8_t * read_buffer, size_t read_size);
};

// Note: 'device' instance is specific for 'Adc' instance.
// FIXME: Move 'device' into 'Adc'
extern Device device;


namespace adc_regs {

struct Config
{
  uint16_t COMP_QUE:2;  // 1..0
    /*
        Comparator queue and disable (ADS1114 and ADS1115 only)

        These bits perform two functions. When set to 11, the comparator is
        disabled and the ALERT/RDY pin is set to a high-impedance state. When
        set to any other value, the ALERT/RDY pin and the comparator function
        are enabled, and the set value determines the number of successive
        conversions exceeding the upper or lower threshold required before
        asserting the ALERT/RDY pin.

        00b : Assert after one conversion
        01b : Assert after two conversions
        10b : Assert after four conversions
        11b : Disable comparator and set ALERT/RDY pin to high-impedance (default)
    */
  uint16_t COMP_LAT:1;  // 2
    /*
        Latching comparator (ADS1114 and ADS1115 only)

        Controls whether the ALERT/RDY pin latches after being asserted or
        clears after conversions are within the margin of the upper and lower
        threshold values.

        0b : Nonlatching comparator. The ALERT/RDY pin does not latch when
        asserted (default).

        1b : Latching comparator. The asserted ALERT/RDY pin remains latched
        until conversion data are read by the controller or an appropriate
        SMBus alert response is sent by the controller. The device responds
        with an address, and is the lowest address currently asserting the
        ALERT/RDY bus line.
    */
  uint16_t COMP_POL:1;  // 3
    /*
        Comparator polarity. Controls polarity of the ALERT/RDY pin.

        0b : Active low (default)
        1b : Active high
    */
  uint16_t COMP_MODE:1; // 4
    /*
        Comparator mode

        0b : Traditional comparator (default)
        1b : Window comparator
    */
  uint16_t DR:3;        // 7..5
    /*
        Data rate

        000b : 8SPS
        001b : 16SPS
        010b : 32SPS
        011b : 64SPS
        100b : 128SPS (default)
        101b : 250SPS
        110b : 475SPS
        111b : 860SPS
    */
  uint16_t MODE:1;      // 8
    /*
        Device operating mode

        0b : Continuous-conversion mode
        1b : Single-shot mode or power-down state (default)
    */
  uint16_t PGA:3;       // 11..9
    /*
        Programmable gain amplifier configuration

        These bits set the FSR of the programmable gain amplifier.

        000b : FSR = ±6.144V(1)
        001b : FSR = ±4.096V(1)
        010b : FSR = ±2.048V (default)
        011b : FSR = ±1.024V
        100b : FSR = ±0.512V
        101b : FSR = ±0.256V
        110b : FSR = ±0.256V
        111b : FSR = ±0.256V
    */
  uint16_t MUX:3;       // 14..12
    /*
        Input multiplexer configuration (ADS1115 only)

        These bits configure the input multiplexer.

        000b : AINP = AIN0 and AINN = AIN1 (default)
        001b : AINP = AIN0 and AINN = AIN3
        010b : AINP = AIN1 and AINN = AIN3
        011b : AINP = AIN2 and AINN = AIN3
        100b : AINP = AIN0 and AINN = GND
        101b : AINP = AIN1 and AINN = GND
        110b : AINP = AIN2 and AINN = GND
        111b : AINP = AIN3 and AINN = GND

        Adafruit_ADS1X15.h:

        CONFIG_MUX_DIFF_0_1 (0x0000) // Differential P = AIN0, N = AIN1 (default)
        CONFIG_MUX_DIFF_0_3 (0x1000) // Differential P = AIN0, N = AIN3
        CONFIG_MUX_DIFF_1_3 (0x2000) // Differential P = AIN1, N = AIN3
        CONFIG_MUX_DIFF_2_3 (0x3000) // Differential P = AIN2, N = AIN3

        CONFIG_MUX_SINGLE_0 (0x4000) // Single-ended AIN0
        CONFIG_MUX_SINGLE_1 (0x5000) // Single-ended AIN1
        CONFIG_MUX_SINGLE_2 (0x6000) // Single-ended AIN2
        CONFIG_MUX_SINGLE_3 (0x7000) // Single-ended AIN3
    */
  uint16_t OS:1;        // 15
    /*
        Operational status or single-shot conversion start

        This bit determines operational status of the device. OS can only be
        written when in power-down state and has no effect when a conversion is
        ongoing.

        When writing:
        0b : No effect
        1b : Start a single conversion (when in power-down state)

        When reading:
        0b : Device is currently performing a conversion.
        1b : Device is not currently performing a conversion.
    */
};

void dump(Config c);

} // namespace adc_regs

struct Adc
{
#if 0
    constexpr uint8_t Conversion = 0; // P[1:0] = 00b, reset = 0000h
    constexpr uint8_t Config     = 1; // P[1:0] = 01b, reset = 8583h
    constexpr uint8_t Lo_thresh  = 2; // P[1:0] = 10b, reset = 8000h
    constexpr uint8_t Hi_thresh  = 3; // P[1:0] = 11b, reset = 7FFFh
#else
  enum class Register
  {
    Conversion = 0, // P[1:0] = 00b, reset = 0000h
    Config     = 1, // P[1:0] = 01b, reset = 8583h
    Lo_thresh  = 2, // P[1:0] = 10b, reset = 8000h
    Hi_thresh  = 3, // P[1:0] = 11b, reset = 7FFFh
  };
#endif

  Device & device_;

  bool init_done_ = false;

  bool init();
  void down();

  bool read(Register rn, uint16_t & v);
  bool read_repeat(uint16_t & v);
  bool write(Register rn, uint16_t v);

  bool start_single_conversion(unsigned channel);
};

extern const i2c_master_bus_config_t i2c_master_bus_config;
extern const i2c_device_config_t i2c_device_config;

extern Adc adc;

} // namespace i2c
} // namespace nc
