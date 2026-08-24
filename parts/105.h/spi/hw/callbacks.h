
// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Request' (MR).
void spi_post_setup_cb(spi_slave_transaction_t * transaction);

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
void spi_post_trans_cb(spi_slave_transaction_t * transaction);

bool spi_transfer_start(uint8_t spi_sensor_n);


extern unsigned spi_post_setup_cb_count_;
extern unsigned spi_post_trans_cb_count_;
