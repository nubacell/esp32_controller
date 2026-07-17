
typedef uint16_t AnalogSignal_t;
#define ANALOG_SIGNALS_COUNT 8
typedef AnalogSignal_t AnalogSignals_t[ANALOG_SIGNALS_COUNT];

typedef uint16_t VCC_t;

typedef struct SensorDeviceData_s {
  uint8_t format; // data format version number, 0 for now
  uint8_t device_id;
  AnalogSignals_t analogSignals;
  VCC_t vcc;
  uint8_t checksum;
} SensorDeviceData_t;
/*
    sizeof(SensorDeviceData_t) on
      LGT8F  21
      ESP32  22 one extra byte is added after checksum
*/
#define LGT8F_SPI_DATA_SIZE OFFSET_OF_END(SensorDeviceData_t, checksum)
