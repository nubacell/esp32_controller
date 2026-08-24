
namespace nc {
namespace spi {
namespace sensors {

extern bool crc_valid_[SPI_SENSORS_COUNT];
extern bool valid_[SPI_SENSORS_COUNT];
extern bool valid_all_;

void validate();

void crc_valid_dump();
void valid_dump();

} // namespace sensors
} // namespace spi
} // namespace nc
