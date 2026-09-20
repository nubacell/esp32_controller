
namespace nc {
namespace spi {

void Sensors::offline_config(uint32_t v)
{
  for (unsigned i = 0; i < SPI_SENSORS_COUNT; ++i) {
    offline_[i] = true;
  }
  //LOG("v=0x%X/%u", v, v);
  while (v) {
    unsigned i = v % 10;
    //LOG("v=0x%X/%u", i, i);
    if (i) {
      --i;
      if (i < SPI_SENSORS_COUNT) {
        offline_[i] = false;
      }
    }
    v /= 10;
  }
}

} // namespace spi
} // namespace nc
