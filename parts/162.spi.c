
//#define DUMP_GPIO_PINS_CONFIG

const uint8_t spi_ms_pins[SPI_SENSOR_DEVICES_COUNT] = {
  SPI_MS_0_GPIO,
  SPI_MS_1_GPIO,
  SPI_MS_2_GPIO,
  SPI_MS_3_GPIO,
  SPI_MS_4_GPIO,
  SPI_MS_5_GPIO,
  SPI_MS_6_GPIO,
  SPI_MS_7_GPIO,
};

byte spi_tx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
uint16_t spi_rx_bits[SPI_SENSOR_DEVICES_COUNT];
uint8_t spi_sensor_n_;

const spi_bus_config_t spi_bus_config = {
  .mosi_io_num = SPI_GPIO_MOSI,
  .miso_io_num = SPI_GPIO_MISO,
  .sclk_io_num = SPI_GPIO_SCLK,
#if 0
  .data2_io_num = -1,
  .data3_io_num = -1,
#else
  .quadwp_io_num = -1,
  .quadhd_io_num = -1,
#endif

  .data4_io_num = -1,
  .data5_io_num = -1,
  .data6_io_num = -1,
  .data7_io_num = -1,

  // Note: One of the following settings results in 'panic':
  // Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
#if 0
  // Output data IO default level when no transaction
  //.data_io_default_level = true,
  //.max_transfer_sz = SPI_BUFFER_SIZE,
  .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#endif
};

#define SPI_QUEUE_SIZE SPI_SENSOR_DEVICES_COUNT

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


// 'MS' pins are 'OUTPUT' with default 'HIGH'/'inactive'

#define SPI_MS_ACTIvE   LOW
#define SPI_MS_INACTIvE HIGH

void spi_ms_pins_setup()
{
  LOG("");
  for (unsigned i = 0; i < ARRAY_SIZE(spi_ms_pins); ++i) {
    const auto spi_ms_pin = spi_ms_pins[i];
    digitalWrite(spi_ms_pin, SPI_MS_INACTIvE);
    pinMode(spi_ms_pin, OUTPUT);
  }
}


const uint8_t spi_pins[] = {
  SPI_GPIO_MOSI,
  SPI_GPIO_SCLK,
  SPI_GPIO_CS,
};

// GPIO_DRIVE_CAP_0: ~5 mA
// GPIO_DRIVE_CAP_1: ~10 mA
// GPIO_DRIVE_CAP_2: ~20 mA -- default
// GPIO_DRIVE_CAP_3: ~40 mA
void spi_pins_setup()
{
  LOG("");
  // MISO pin (if used) is 'OUTPUT'.

  // MOSI, SCLK and CS pins are 'INPUT_PULLUP'
  for (unsigned i = 0; i < ARRAY_SIZE(spi_pins); ++i) {
    uint8_t spi_pin = spi_pins[i];
    pinMode(spi_pin, INPUT_PULLUP);
    //gpio_set_pull_mode(spi_pin, GPIO_PULLUP_ONLY);
    //gpio_set_drive_capability(spi_pin, GPIO_DRIVE_CAP_3);
  }
}


// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Select' (MS).
unsigned spi_post_setup_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_setup_cb_us_;
#endif
void spi_post_setup_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_setup_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_setup_cb_us_ = esp_timer_get_time();
#endif

  //spi_sensor_n_ %= SPI_SENSOR_DEVICES_COUNT;
  gpio_set_level(gpio_num_t(spi_ms_pins[spi_sensor_n_]), 0);
  //digitalWrite(spi_ms_pins[spi_sensor_n_], SPI_MS_ACTIvE);
}

//volatile
bool spi_transfer_finished_;

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Select' (MS).
// Can be used to mark end of multiple transfers.
unsigned spi_post_trans_cb_count_;
#if defined SPI_TRANS_TIME_STATS
uint32_t spi_post_trans_cb_us_;
uint32_t spi_post_trans_cb_us_min_ = uint32_t(-1);
uint32_t spi_post_trans_cb_us_max_;
#endif
void spi_post_trans_cb(spi_slave_transaction_t * transaction)
{
  ++spi_post_trans_cb_count_;

#if defined SPI_TRANS_TIME_STATS
  spi_post_trans_cb_us_ = esp_timer_get_time();
  uint32_t trans_us = spi_post_trans_cb_us_ - spi_post_setup_cb_us_;
  if (spi_post_trans_cb_us_min_ > trans_us) {
    spi_post_trans_cb_us_min_ = trans_us;
  }
  if (spi_post_trans_cb_us_max_ < trans_us) {
    spi_post_trans_cb_us_max_ = trans_us;
  }
#endif

  //spi_sensor_n_ %= SPI_SENSOR_DEVICES_COUNT;
  //digitalWrite(spi_ms_pins[spi_sensor_n_], SPI_MS_INACTIvE);
  gpio_set_level(gpio_num_t(spi_ms_pins[spi_sensor_n_]), 1);

  spi_rx_bits[spi_sensor_n_] = transaction->trans_len;

  spi_transfer_finished_ = true;
}


static bool spi_init_failure = false;

void spi_setup()
{
  LOG("");
  spi_pins_setup();
  spi_ms_pins_setup();

  spi_init_failure = (ESP_OK != spi_slave_initialize(SPI_DEVICE,
      &spi_bus_config, &spi_slave_interface_config, SPI_DMA_DISABLED));
  if (spi_init_failure) {
    LOG("ERROR: %s() failure", "spi_slave_initialize");
  }
  LOG("");

#if defined DUMP_GPIO_PINS_CONFIG
  gpio_dump_io_configuration(stdout,
      (1ULL << 12)
      | (1ULL << 13)
      | (1ULL << 14)
      | (1ULL << 15)
      | (1ULL << 22)
      | (1ULL << 23));
#endif
}


/*
    Warning: 'spi_slave_queue_trans()' does not make copy of
    'spi_slave_transaction_t'. Instead it keeps it's address and uses that
    address to store number of actually transferred bits into 'trans_len'
    field. It means that memory occupied by 'spi_slave_transaction_t' must be
    preserved until the end of transmission which is notified by
    'post_trans_cb' callback.
*/
static spi_slave_transaction_t spi_transaction_;
bool spi_transfer_start(uint8_t spi_sensor_n)
{
  spi_transfer_finished_ = false;

  spi_transaction_ = spi_slave_transaction_t {
    //.flags = 0,
    .length = SPI_DATA_SIZE * 8 + 6,
    // trans_len -- used by driver to return actual number of received bits
    //.trans_len = 0,
    .tx_buffer = &spi_tx_buffers[spi_sensor_n][0],
    .rx_buffer = &spi_rx_buffers[spi_sensor_n][0],
    // void * user // User-defined variable.
    // Can be used to store MS for 'spi_post_setup_cb' and 'spi_post_trans_cb'.
    //.user = NULL,
  };
  if (ESP_OK != spi_slave_queue_trans(SPI_DEVICE, &spi_transaction_, 0)) {
    LOG("ERROR: %s() failure", "spi_slave_queue_trans");
    return false;
  }
#if 0
  LOG(". spi_post_setup_cb_count_=%u spi_post_trans_cb_count_=%u",
      spi_post_setup_cb_count_, spi_post_trans_cb_count_);
#endif
  return true;
}


#define SPI_RX_BITS_MIN (SPI_DATA_SIZE * 8)

void spi_rx_bits_reset()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    spi_rx_bits[i] = 0;
  }
}

bool spi_rx_bits_valid()
{
  for (unsigned i = 0; i < ARRAY_SIZE(spi_rx_bits); ++i) {
    if (spi_rx_bits[i] < SPI_RX_BITS_MIN) {
      LOG(" . spi_rx_bits[%u]=%u SPI_RX_BITS_MIN=%u", i, spi_rx_bits[i], SPI_RX_BITS_MIN);
      return false;
    }
  }
  return true;
}

SPI_Device spiDevice;

const char * SPI_Device::to_string(State state)
{
  static const char * const state_names[] = {
    "init",
    "init_failed",
    "idle",
    "running",
  };
  const auto i = unsigned(state);
  return (i < ARRAY_SIZE(state_names)) ? state_names[i] : "?";
}

void SPI_Device::state(State new_state)
{
  const auto old_state_name = to_string(state_);
  state_ = new_state;
  const auto new_state_name = to_string(state_);
  LOG("'%s' -> '%s'", old_state_name, new_state_name);
}

void SPI_Device::on_state_init(uint32_t now_ms)
{
  (void) now_ms;
  spi_setup();
  state( (spi_init_failure) ? State::init_failed : State::idle);
}

// Can be used to implement delay before init retry
void SPI_Device::on_state_init_failed(uint32_t now_ms)
{
  (void) now_ms;
}

void SPI_Device::on_state_idle(uint32_t now_ms)
{
  (void) now_ms;
  if (run_) {
    run_ = false;

    constexpr uint32_t spi_transfer_all_timeout_ms = 100;
    timeout_time_ms_ = now_ms + spi_transfer_all_timeout_ms;

    spi_rx_bits_reset();
    spi_sensor_n_ = 0;
    sensors_data_valid_ = false;

    memset(spi_rx_buffers, 0, sizeof(spi_rx_buffers));
    //memset(spi_rx_buffers, 0x5A, sizeof(spi_rx_buffers));
    //memset(spi_rx_buffers, 'W', sizeof(spi_rx_buffers));

    if (spi_transfer_start(spi_sensor_n_)) {
      state(State::running);
    } else {
      state(State::idle);
    }
  }
}

void SPI_Device::on_state_running(uint32_t now_ms)
{
  if (spi_transfer_finished_) {
    //LOG("spi_transfer_finished_");

    //LOG(". spi_rx_buffers");
    //hex_dump( (const uint8_t*)spi_rx_buffers, 32);

    if (++spi_sensor_n_ < SPI_SENSOR_DEVICES_COUNT) {
      //LOG("spi_sensor_n_=%u < SPI_SENSOR_DEVICES_COUNT=%u", spi_sensor_n_, SPI_SENSOR_DEVICES_COUNT);
      if (!spi_transfer_start(spi_sensor_n_)) {
        LOG("");
        state(State::idle);
      }
    } else {
      //LOG("end of sensors");
      sensors_data_valid_ = spi_rx_bits_valid();
      //LOG("sensors_data_valid_=%i", sensors_data_valid_);
      state(State::idle);
    }
  } else
  if (timeout_time_ms_ <= now_ms) {
    LOG("%s %s", "spi transfer", "timeout");

    // make sure MS pin of spi_sensor_n_ is inactive
    digitalWrite(spi_ms_pins[spi_sensor_n_], SPI_MS_INACTIvE);

    state(State::idle);
  }
}

void SPI_Device::on_loop(uint32_t now_ms)
{
  switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
    CASE_CALL(init)
    CASE_CALL(init_failed)
    CASE_CALL(idle)
    CASE_CALL(running)
#undef CASE_CALL
    default:
      //LOG("ERROR: Unexpected state=%u", state_);
      LOG("ERROR: Unexpected state=%u/'%s'", state_, to_string(state_));
      break;
  }
}

void spi_run(const char * args)
{
  (void) args;
  spiDevice.run();
}

void spi_report_config()
{
  LOG(". %s=%u", "SPI_GPIO_MISO", SPI_GPIO_MISO);
  LOG(". %s=%u", "SPI_GPIO_MOSI", SPI_GPIO_MOSI);
  LOG(". %s=%u", "SPI_GPIO_SCLK", SPI_GPIO_SCLK);
  LOG(". %s=%u", "SPI_GPIO_CS", SPI_GPIO_CS);
  for (unsigned i = 0; i < ARRAY_SIZE(spi_ms_pins); ++i) {
    const auto spi_ms_pin = spi_ms_pins[i];
    LOG(". SPI_MS_%u_GPIO=%u", i, spi_ms_pin);
  }
  LOG(". %s=%u", "SPI_BUFFER_SIZE", SPI_BUFFER_SIZE);
  LOG(". %s=%u", "SPI_DATA_SIZE", SPI_DATA_SIZE);
  LOG(". %s=%u", "SPI_RX_BITS_MIN", SPI_RX_BITS_MIN);
  LOG(". %s=%u", "sizeof(spi_rx_buffers)", sizeof(spi_rx_buffers));
}

void spi_report_stats()
{
  LOG(". spi_post_setup_cb_count_=%u spi_post_trans_cb_count_=%u",
      spi_post_setup_cb_count_, spi_post_trans_cb_count_);
#if defined SPI_TRANS_TIME_STATS
  LOG(". spi_post_trans_cb_us_min_=%u spi_post_trans_cb_us_max_=%u",
      spi_post_trans_cb_us_min_, spi_post_trans_cb_us_max_);
#endif

  for (unsigned i = 0; i < SPI_SENSOR_DEVICES_COUNT; ++i) {
    hex_dump(&spi_rx_buffers[i][0], SPI_DATA_SIZE);
  }
}
