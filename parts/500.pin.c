
void pin_dump(const char * args)
{
  unsigned pin = command_args_to_ul(args, nullptr);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }
  gpio_dump_io_configuration(stdout, (1ULL << pin));
}

/*
    esp_err_t gpio_reset_pin(gpio_num_t gpio_num)

      gpio_intr_disable(gpio_num);
      gpio_pullup_en(gpio_num);
      gpio_pulldown_dis(gpio_num);
      gpio_input_disable(gpio_num);
      gpio_output_disable(gpio_num);
      gpio_hal_func_sel(gpio_context.gpio_hal, gpio_num, PIN_FUNC_GPIO);

*/
void pin_reset(const char * args)
{
  unsigned pin = command_args_to_ul(args, nullptr);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }
  if (ESP_OK != gpio_reset_pin(gpio_num_t(pin))) {
    LOG("ERROR: %s(pin=%u)", "gpio_reset_pin", pin);
  }
}

/*
    int gpio_get_level(gpio_num_t gpio_num);
    esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level);

      level Output level. 0: low ; 1: high
*/
void pin_level(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    unsigned v = command_args_to_ul(tail, nullptr);
    LOG(". setting pin=%u level=%u", pin, v);
    if (ESP_OK != gpio_set_level(gpio_num_t(pin), v)) {
      LOG("ERROR: %s(pin=%u)", "gpio_set_level", pin);
    }
  }

  LOG("%s(pin=%u)=%u", "gpio_get_level", pin, gpio_get_level(gpio_num_t(pin)));
}

/*
    esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode);

    gpio_mode_t

      GPIO_MODE_DISABLE
      GPIO_MODE_INPUT                   // (1<<0)
      GPIO_MODE_OUTPUT                  //          (1<<1)
      GPIO_MODE_OUTPUT_OD               //          (1<<1) | (1<<2)
      GPIO_MODE_INPUT_OUTPUT            // (1<<0) | (1<<1)
      GPIO_MODE_INPUT_OUTPUT_OD         // (1<<0) | (1<<1) | (1<<2)

    if (mode & GPIO_MODE_DEF_INPUT ) { gpio_input_enable(gpio_num);  } else { gpio_input_disable(gpio_num); }
    if (mode & GPIO_MODE_DEF_OUTPUT) { gpio_output_enable(gpio_num); } else { gpio_output_disable(gpio_num); }
    if (mode & GPIO_MODE_DEF_OD    ) { gpio_od_enable(gpio_num);     } else { gpio_od_disable(gpio_num); }

*/
static const struct {
  gpio_mode_t mode;
  const char * name;
} pin_modes[] = {
#define MODE(x) { GPIO_MODE_##x, #x }
  MODE(DISABLE),
  MODE(INPUT),
  MODE(OUTPUT),
  MODE(OUTPUT_OD),
  MODE(INPUT_OUTPUT_OD),
  MODE(INPUT_OUTPUT),
#undef MODE
};

static void pin_modes_list()
{
  for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
    LOG(". %u %s", pin_modes[i].mode, pin_modes[i].name);
  }
}

void pin_mode(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    LOG("tail='%s'", tail);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
      if (!strcasecmp(tail, pin_modes[i].name)) {
        LOG(". setting mode %u %s", pin_modes[i].mode, pin_modes[i].name);
        const auto mode = pin_modes[i].mode;
        if (ESP_OK != gpio_set_direction(gpio_num_t(pin), mode)) {
          LOG("ERROR: %s(pin=%u, mode=%u)", "gpio_set_direction", pin, mode);
        }
        return;
      }
    }
    unsigned v = command_args_to_ul(tail, nullptr);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_modes); ++i) {
      if (pin_modes[i].mode == v) {
        LOG(". setting mode %u %s", pin_modes[i].mode, pin_modes[i].name);
        const auto mode = pin_modes[i].mode;
        if (ESP_OK != gpio_set_direction(gpio_num_t(pin), mode)) {
          LOG("ERROR: %s(pin=%u, mode=%u)", "gpio_set_direction", pin, mode);
        }
        return;
      }
    }
  }

  LOG("pin modes:");
  pin_modes_list();
}

/*
    esp_err_t gpio_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull);

      gpio_pull_mode_t

        GPIO_PULLUP_ONLY,               // Pad pull up
        GPIO_PULLDOWN_ONLY,             // Pad pull down
        GPIO_PULLUP_PULLDOWN,           // Pad pull up + pull down
        GPIO_FLOATING,                  // Pad floating
*/

static const struct {
  const char * name;
  gpio_pull_mode_t pull;
} pin_pulls[] = {
#define PULL(n, e) { n, GPIO_##e }
  PULL("UP"  , PULLUP_ONLY),
  PULL("DOWN", PULLDOWN_ONLY),
  PULL("BOTH", PULLUP_PULLDOWN),
  PULL("NONE", FLOATING),
#undef MODE
};

static void pin_pulls_list()
{
  for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
    LOG(". %u %s", pin_pulls[i].pull, pin_pulls[i].name);
  }
}

void pin_pull(const char * args)
{
  const char * tail = NULL;
  unsigned pin = command_args_to_ul(args, &tail);
  if (pin >= GPIO_NUM_MAX) {
    LOG("ERROR: pin=%u must be < GPIO_NUM_MAX=%u", pin, GPIO_NUM_MAX);
    return;
  }

  if (tail && *tail) {
    LOG("tail='%s'", tail);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
      if (!strcasecmp(tail, pin_pulls[i].name)) {
        LOG(". setting pull %u %s", pin_pulls[i].pull, pin_pulls[i].name);
        const auto pull = pin_pulls[i].pull;
        if (ESP_OK != gpio_set_pull_mode(gpio_num_t(pin), pull)) {
          LOG("ERROR: %s(pin=%u, pull=%u)", "gpio_set_pull_mode", pin, pull);
        }
        return;
      }
    }
    unsigned v = command_args_to_ul(tail, nullptr);
    for (unsigned i = 0; i < ARRAY_SIZE(pin_pulls); ++i) {
      if (pin_pulls[i].pull == v) {
        LOG(". setting pull %u %s", pin_pulls[i].pull, pin_pulls[i].name);
        const auto pull = pin_pulls[i].pull;
        if (ESP_OK != gpio_set_pull_mode(gpio_num_t(pin), pull)) {
          LOG("ERROR: %s(pin=%u, pull=%u)", "gpio_set_pull_mode", pin, pull);
        }
        return;
      }
    }
  }

  LOG("pin pulls:");
  pin_pulls_list();
}

/*
    esp_err_t gpio_input_enable(gpio_num_t gpio_num);
    esp_err_t gpio_output_enable(gpio_num_t gpio_num);
    esp_err_t gpio_output_disable(gpio_num_t gpio_num);
    esp_err_t gpio_od_disable(gpio_num_t gpio_num)
    esp_err_t gpio_od_enable(gpio_num_t gpio_num)

    'gpio_input_disable' exists but is inaccessible due to 'static':
    static esp_err_t gpio_input_disable(gpio_num_t gpio_num)
*/
