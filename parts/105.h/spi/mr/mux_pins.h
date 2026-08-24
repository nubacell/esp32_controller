
// 'Master Request' (MR) multiplexer (MUX) is used to select one of many LGT8F
// devices
#define SPI_MR_MUX_GPIO_COUNT 4
#define SPI_MR_MUX_0_GPIO 32
#define SPI_MR_MUX_1_GPIO 33
#define SPI_MR_MUX_2_GPIO 19
#define SPI_MR_MUX_3_GPIO 18

extern const uint8_t spi_mr_mux_pins[SPI_MR_MUX_GPIO_COUNT];
