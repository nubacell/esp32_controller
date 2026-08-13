
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html

//#define I2C_TIME_STATS

constexpr uint32_t I2C_TIMEOUT_MS = 2;

namespace nc {
namespace i2c {

const i2c_master_bus_config_t i2c_master_bus_config = {
  .i2c_port = 0, // I2C_MASTER_NUM,
  .sda_io_num = gpio_num_t(21),
  .scl_io_num = gpio_num_t(22),
  .clk_source = I2C_CLK_SRC_DEFAULT,
  .glitch_ignore_cnt = 7,
  //.trans_queue_depth = 1, // must '>0' for 'async' mode
  //.flags = { .enable_internal_pullup = true, },
  .flags = { .enable_internal_pullup = false, },
};

Bus bus = {
  .i2c_master_bus_config_ = i2c_master_bus_config,
};

// address must be 0x49 (default is 0x48)
#define i2c_adc_address 0x49

// 400kHz is actually 384kHz
#define i2c_adc_frequency 400000
//#define i2c_adc_frequency 650000
// at 675kHz i2c adc frequently fails

const i2c_device_config_t i2c_device_config = {
  .dev_addr_length = I2C_ADDR_BIT_LEN_7,
  .device_address = i2c_adc_address,
  .scl_speed_hz = i2c_adc_frequency,
};

Device device = {
  .bus_ = bus,
  .i2c_device_config_ = i2c_device_config,
};

/*
    esp_err_t i2c_new_master_bus(
        const i2c_master_bus_config_t * bus_config,
        i2c_master_bus_handle_t * ret_bus_handle)

    const char * esp_err_to_name(esp_err_t code)
*/
bool Bus::init()
{
  if (init_done_) { return true; }

  const auto e = i2c_new_master_bus(
      &i2c_master_bus_config_, &i2c_master_bus_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_new_master_bus", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = true;
  }
  return r;
}

/*
    esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus_handle)
*/
void Bus::down()
{
  if (!init_done_) { return; }

  const auto e = i2c_del_master_bus(i2c_master_bus_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_del_master_bus", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = false;
  }
}

/*
    esp_err_t i2c_master_probe(
        i2c_master_bus_handle_t bus_handle,
        uint16_t address,
        int xfer_timeout_ms)
*/
bool Bus::probe(uint16_t address)
{
  const auto e = i2c_master_probe(
      i2c_master_bus_handle_, address, I2C_TIMEOUT_MS);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_probe", "failure", e, esp_err_to_name(e));
  }
  return r;
}


/*
    esp_err_t i2c_master_bus_add_device(
        i2c_master_bus_handle_t bus_handle,
        const i2c_device_config_t * dev_config,
        i2c_master_dev_handle_t * ret_handle)
*/
bool Device::init()
{
  if (init_done_) { return true; }

  const auto e = i2c_master_bus_add_device(
      bus_.i2c_master_bus_handle_,
      &i2c_device_config, &i2c_master_dev_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_bus_add_device", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = true;
  }
  return r;
}

/*
    esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t handle)
*/
void Device::down()
{
  if (!init_done_) { return; }

  const auto e = i2c_master_bus_rm_device(i2c_master_dev_handle_);
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_bus_rm_device", "failure", e, esp_err_to_name(e));
  } else {
    init_done_ = false;
  }
}

/*
    esp_err_t i2c_master_receive(
        i2c_master_dev_handle_t i2c_dev,
        uint8_t * read_buffer, size_t read_size,
        int xfer_timeout_ms)
*/
bool Device::read(uint8_t * read_buffer, size_t read_size)
{
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_receive(
      i2c_master_dev_handle_,
      read_buffer, read_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "i2c_master_receive", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_receive", "failure", e, esp_err_to_name(e));
  } else {
    //hex_dump(read_buffer, read_size);
  }
  return r;
}

/*
    esp_err_t i2c_master_transmit(
        i2c_master_dev_handle_t i2c_dev,
        const uint8_t * write_buffer, size_t write_size,
        int xfer_timeout_ms)
*/
bool Device::write(const uint8_t * write_buffer, size_t write_size)
{
  //hex_dump(write_buffer, write_size);
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_transmit(
      i2c_master_dev_handle_,
      write_buffer, write_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "i2c_master_transmit", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_transmit", "failure", e, esp_err_to_name(e));
  }
  return r;
}

/*
    esp_err_t i2c_master_transmit_receive(
        i2c_master_dev_handle_t i2c_dev,
        const uint8_t *write_buffer, size_t write_size,
        uint8_t *read_buffer, size_t read_size,
        int xfer_timeout_ms)
*/
bool Device::write_read(
      const uint8_t * write_buffer, size_t write_size,
      uint8_t * read_buffer, size_t read_size)
{
  //hex_dump(write_buffer, write_size);
#if defined I2C_TIME_STATS
  uint64_t us1 = esp_timer_get_time();
#endif
  const auto e = i2c_master_transmit_receive(
      i2c_master_dev_handle_,
      write_buffer, write_size,
      read_buffer, read_size,
      I2C_TIMEOUT_MS);
#if defined I2C_TIME_STATS
  uint64_t us2 = esp_timer_get_time();
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us2, us1, us2 - us1);
#endif
  const auto r = (ESP_OK == e);
  if (!r) {
    LOG("'%s()' %s %i '%s'", "i2c_master_transmit_receive", "failure", e, esp_err_to_name(e));
  } else {
    //hex_dump(read_buffer, read_size);
  }
  return r;
}


bool Adc::read(Register rn, uint16_t & v)
{
  const uint8_t write_data[1] = { uint8_t(rn) };
  uint8_t read_data[2] = { 0, 0 };
  const auto r = device_.write_read(
      write_data, sizeof(write_data),
      read_data, sizeof(read_data));
  if (r) {
    v = (uint16_t(read_data[0]) << 8) | read_data[1];
    //LOG("rn=%u v=0x%04X/%u/%i", unsigned(rn), v, v, int16_t(v));
  }
  return r;
}

bool Adc::read_repeat(uint16_t & v)
{
  uint8_t read_data[2] = { 0, 0 };
  const auto r = device_.read(read_data, sizeof(read_data));
  if (r) {
    v = (uint16_t(read_data[0]) << 8) | read_data[1];
    //LOG("v=0x%04X/%u/%i", v, v, int16_t(v));
  }
  return r;
}

bool Adc::write(Register rn, uint16_t v)
{
  const uint8_t write_data[3] = { uint8_t(rn), v >> 8, v & 0xFF };
  const auto r = device_.write(write_data, sizeof(write_data));
#if 0
  if (r) {
    LOG("rn=%u v=0x%04X/%u/%i", unsigned(rn), v, v, int16_t(v));
  }
#endif
  return r;
}


namespace adc_regs {

void dump(Config c)
{
#if 1
  {
    const auto v = *(uint16_t*) &c;
    LOG("c=0x%04X/%u/%i", v, v, int16_t(v));
  }
#endif

  static const char * const dr_names[] = {
    "8", "16", "32", "64", "128", "250", "475", "860"
  };

  LOG(
    "OS=%u"
    " MUX=%u"
    " PGA=%u"
    " MODE=%u"
    " DR=%u/'%s'"
    " COMP_MODE=%u"
    " COMP_POL=%u"
    " COMP_LAT=%u"
    " COMP_QUE=%u",
    c.OS,
    c.MUX,
    c.PGA,
    c.MODE,
    c.DR, dr_names[c.DR],
    c.COMP_MODE,
    c.COMP_POL,
    c.COMP_LAT,
    c.COMP_QUE
  );
}

} // namespace adc_regs


Adc adc = {
  .device_ = device,
};

bool Adc::init()
{
  if (init_done_) { return true; }

  if (!bus.init()) { return false; }
  if (!device.init()) {
    bus.down();
    return false;
  }
  init_done_ = true;
  return true;
}

void Adc::down()
{
  if (!init_done_) { return; }

  device.down();
  bus.down();

  init_done_ = false;
}

} // namespace i2c
} // namespace nc

void i2c_probe(const char * args)
{
  unsigned device = command_args_to_ul(args, nullptr);
  LOG("device=0x%X/%u", device, device);
  if (!::nc::i2c::bus.init_done_) {
    if (!::nc::i2c::bus.init()) { return; }
  }
  const auto probe_r = ::nc::i2c::bus.probe(device);
  LOG("probe_r=%i", probe_r);
}

void i2c_adc_read(const char * args)
{
  unsigned register_number = command_args_to_ul(args, nullptr);
  LOG("register_number=0x%X/%u", register_number, register_number);
  if (register_number >= 4) {
    LOG("ERROR: register_number must be < 4");
    return;
  }
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register(register_number), v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}

void i2c_adc_read_repeat(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }
  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read_repeat(v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
}

void i2c_adc_write(const char * args)
{
  const char * tail = NULL;
  unsigned register_number = command_args_to_ul(args, &tail);
  LOG("register_number=0x%X/%u", register_number, register_number);
  if (register_number >= 4) {
    LOG("ERROR: register_number must be < 4");
    return;
  }

  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  if (tail && *tail) {
    unsigned v = command_args_to_ul(tail, nullptr);
    const auto r = ::nc::i2c::adc.write(::nc::i2c::Adc::Register(register_number), v);
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }

  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register(register_number), v);
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }
}

void i2c_adc_config(const char * args)
{
  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint16_t v = 0;
  const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
  LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  ::nc::i2c::adc_regs::dump( *(::nc::i2c::adc_regs::Config*) &v);
}

void i2c_adc_single(const char * args)
{
  unsigned channel = command_args_to_ul(args, nullptr);
  LOG("channel=%u", channel);
  if (channel >= 4) {
    LOG("ERROR: channel must be < 4");
    return;
  }

  if (!::nc::i2c::adc.init_done_ && !::nc::i2c::adc.init()) { return; }

  uint64_t us1 = esp_timer_get_time();
  {
    const auto r = ::nc::i2c::adc.start_single_conversion(channel);
    //LOG("r=%i", r);
    if (!r) { return; }
  }
  uint64_t us2 = esp_timer_get_time();
  unsigned wait_count = 0;
  bool converting;
  do {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read_repeat(v);
    //LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
    if (!r) { return; }
    ++wait_count;
    if (wait_count > 100) {
      LOG("wait_count limit exceeded");
      break;
    }
    const auto c = *(::nc::i2c::adc_regs::Config*) &v;
    converting = !c.OS;
  } while (converting);
  uint64_t us3 = esp_timer_get_time();
#if 0
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Config, v);
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i", r, v, v, int16_t(v));
  }
#endif
  uint64_t us4;
  {
    uint16_t v = 0;
    const auto r = ::nc::i2c::adc.read(::nc::i2c::Adc::Register::Conversion, v);
    us4 = esp_timer_get_time();
    if (!r) { return; }
    LOG("r=%i v=0x%04X/%u/%i '%s'", r, v, v, int16_t(v), "Conversion");
  }
  LOG("%s us2=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us2, us1, us2 - us1);
  LOG("%s us3=%" PRIu64 " - us2=%" PRIu64 " =%" PRIu64, "write_read", us3, us2, us3 - us2);
  LOG("%s us4=%" PRIu64 " - us3=%" PRIu64 " =%" PRIu64, "write_read", us4, us3, us4 - us3);
  LOG("%s us4=%" PRIu64 " - us1=%" PRIu64 " =%" PRIu64, "write_read", us4, us1, us4 - us1);
  LOG("wait_count=%u", wait_count);
}
