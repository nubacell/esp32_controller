
#if 1
static const UartConfig * const xx_uartConfigs[] = {
  &cn_uartConfig,
  &ps_uartConfig,
  &el_uartConfig,
};

static void xx_uartConfigs_init() {
  for (unsigned i = 0; i < ARRAY_SIZE(xx_uartConfigs); ++i) {
    xx_uartConfigs[i]->init();
  }
}
#endif
