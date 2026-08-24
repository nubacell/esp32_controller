
/*
    Currently only one transaction is used.

    Multiple transactions submitted to driver can improve performance but will
    complicate cleanup in case of timeout.
*/
//#define SPI_QUEUE_SIZE SPI_SENSORS_COUNT
#define SPI_QUEUE_SIZE 1

/*
    SPI mode is combination of CPOL and CPHA:
      0: (0, 0)
      1: (0, 1)
      2: (1, 0)
      3: (1, 1)
*/
const spi_slave_interface_config_t spi_slave_interface_config = {
  .spics_io_num  = SPI_GPIO_CS,
  .flags         = SPI_SLAVE_NO_RETURN_RESULT, // suppresses results queue creation
  .queue_size    = SPI_QUEUE_SIZE,
  .mode          = 0,
  .post_setup_cb = spi_post_setup_cb,
  .post_trans_cb = spi_post_trans_cb
};
