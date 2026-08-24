
namespace nc {
namespace spi {
namespace sensors {

extern bool offline_[SPI_SENSORS_COUNT];

uint8_t skip_offline(uint8_t spi_sensor_n);

void config(uint32_t v);
void config_dump();

} // namespace sensors
} // namespace spi
} // namespace nc
