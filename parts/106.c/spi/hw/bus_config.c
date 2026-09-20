
const spi_bus_config_t spi_bus_config = {
  .mosi_io_num = SPI_GPIO_MOSI,
  .miso_io_num = SPI_GPIO_MISO,
  .sclk_io_num = SPI_GPIO_SCLK,

  .quadwp_io_num = -1,
  .quadhd_io_num = -1,

  .data4_io_num = -1,
  .data5_io_num = -1,
  .data6_io_num = -1,
  .data7_io_num = -1,

#if 0
  // Output data IO default level when no transaction
  // Note: Looks like ESP32 'slave' does not support 'data_io_default_level'
  //.data_io_default_level = true,
  //.max_transfer_sz = SPI_BUFFER_SIZE,
  //.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#endif
};
