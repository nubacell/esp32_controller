
// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Request' (MR).
// Can be used to mark end of multiple transfers.
void spi_post_trans_cb(spi_slave_transaction_t * transaction);

extern unsigned spi_post_trans_cb_count_;
