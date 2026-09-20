
namespace nc {
namespace lgt {

using VCC = uint16_t;

using Voltage = int16_t;
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const Voltage voltage);

static constexpr const unsigned voltage_pins_COUNT = 4;
using Voltages = Voltage[voltage_pins_COUNT];
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const ::nc::lgt::Voltages & voltages);

using Temperature = uint16_t;
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const Temperature temperature);

static constexpr const unsigned temperature_pins_COUNT = 4;
static constexpr const unsigned temperature_groups_COUNT = 2;
using Temperatures = Temperature[temperature_pins_COUNT][temperature_groups_COUNT];
::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b, const ::nc::lgt::Temperatures & temperatures);

static constexpr const unsigned ALL_ANALOG_SIGNALS_COUNT =
    voltage_pins_COUNT + temperature_pins_COUNT * temperature_groups_COUNT;
using AllAnalogSignals = int16_t[ALL_ANALOG_SIGNALS_COUNT];

union AnalogSignals
{
  struct
  {
    Voltages voltages;
    Temperatures temperatures;
  };
  AllAnalogSignals all;

  ::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b) const noexcept;
};

struct SensorData
{
  uint8_t format; // data format version number, 1 for now
  uint8_t device_id;
  AnalogSignals analogSignals;
  VCC vcc;
  uint8_t crc;

  ::nc::mk::Buffer & to_buffer(::nc::mk::Buffer & b) const noexcept;
};

} // namespace lgt
} // namespace nc

/*
    sizeof(SensorData) on
      LGT8F  21
      ESP32  22 one extra byte is added after crc
*/
#define LGT8F_SPI_DATA_SIZE OFFSET_OF_END(::nc::lgt::SensorData, crc)

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Voltages & voltages)
{
  return ::nc::lgt::to_buffer(b, voltages);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::Temperatures & temperatures)
{
  return ::nc::lgt::to_buffer(b, temperatures);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::AnalogSignals & analogSignals)
{
  return analogSignals.to_buffer(b);
}

static
inline
::nc::mk::Buffer & operator<<(
    ::nc::mk::Buffer & b,
    const ::nc::lgt::SensorData & sensorData)
{
  return sensorData.to_buffer(b);
}
