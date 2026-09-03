
#define voltage_pins_count 4
#define temperature_pins_count 4
#define temperature_groups_count 2

typedef int16_t AnalogSignal_t;
#define ANALOG_SIGNALS_COUNT 12
typedef union AnalogSignals_u {
  struct {
    AnalogSignal_t voltage[voltage_pins_count];
    AnalogSignal_t temperature[temperature_pins_count][temperature_groups_count];
  };
  AnalogSignal_t all[ANALOG_SIGNALS_COUNT];
} AnalogSignals_t;

typedef uint16_t VCC_t;

typedef struct SensorDeviceData_s {
  uint8_t format; // data format version number, 1 for now
  uint8_t device_id;
  AnalogSignals_t analogSignals;
  VCC_t vcc;
  uint8_t crc;
} SensorDeviceData_t;
/*
    sizeof(SensorDeviceData_t) on
      LGT8F  21
      ESP32  22 one extra byte is added after crc
*/
#define LGT8F_SPI_DATA_SIZE OFFSET_OF_END(SensorDeviceData_t, crc)
