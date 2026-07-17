
#define SPI_GPIO_MISO 12
#define SPI_GPIO_MOSI 13
#define SPI_GPIO_SCLK 14
#define SPI_GPIO_CS   15

#define SPI_DEVICE SPI2_HOST

#define SPI_BUFFER_SIZE_MAX 64
#define SPI_BUFFER_SIZE SPI_BUFFER_SIZE_MAX
//#define SPI_BUFFER_SIZE 128
#define SPI_DATA_SIZE 21


#define SPI_SENSOR_DEVICES_COUNT 8

// 'Master Select' (MS) is used to select one of many LGT8F devices
#define SPI_MS_0_GPIO 22
#define SPI_MS_1_GPIO 23
// FIXME: Assign correct pin numbers
#define SPI_MS_2_GPIO SPI_MS_0_GPIO
#define SPI_MS_3_GPIO SPI_MS_1_GPIO
#define SPI_MS_4_GPIO SPI_MS_0_GPIO
#define SPI_MS_5_GPIO SPI_MS_0_GPIO
#define SPI_MS_6_GPIO SPI_MS_1_GPIO
#define SPI_MS_7_GPIO SPI_MS_1_GPIO


extern const uint8_t spi_ms_pins[SPI_SENSOR_DEVICES_COUNT];
extern byte spi_tx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
extern byte spi_rx_buffers[SPI_SENSOR_DEVICES_COUNT][SPI_BUFFER_SIZE];
extern uint16_t spi_rx_bits[SPI_SENSOR_DEVICES_COUNT];

extern const spi_bus_config_t spi_bus_config;
extern const spi_slave_interface_config_t spi_slave_interface_config;


// Called after SPI is configured for 'transaction'.
// Can be used to activate 'Master Select' (MS).
void spi_post_setup_cb(spi_slave_transaction_t * transaction);

// Called after 'transaction' is sent/received.
// Can be used to deactivate 'Master Select' (MS).
// Can be used to mark end of multiple transfers.
void spi_post_trans_cb(spi_slave_transaction_t * transaction);

void spi_setup();

void spi_ms_pins_setup();
void spi_pins_setuo();

bool spi_transfer_start(uint8_t spi_sensor_n);

void spi_rx_bits_reset();
bool spi_rx_bits_valid();


struct SPI_Device
  : Runnable
{
  uint32_t timeout_time_ms_;

  bool run_ = false;
  bool sensors_data_valid_ = false;

  enum class State {
    init,
    init_failed,
    idle,
    running,
  };
  State state_ = State::init;
  static const char * to_string(State state);
  void state(State new_state);

  void on_state_init(uint32_t now_ms);
  void on_state_init_failed(uint32_t now_ms);
  void on_state_idle(uint32_t now_ms);
  void on_state_running(uint32_t now_ms);

  void on_loop(uint32_t now_ms) override;

  void run()
  {
    run_ = true;
  }

  bool isTransferFinished() { return State::idle == state_; }
  bool areSensorsDataValid() { return sensors_data_valid_; }
};

extern SPI_Device spiDevice;


extern unsigned spi_post_setup_cb_count_;
extern unsigned spi_post_trans_cb_count_;

void spi_report_stats();

void spi_run(const char * args);
void spi_stats(const char * args);
