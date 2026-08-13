
namespace nc {
namespace i2c {

bool Adc::start_single_conversion(unsigned channel)
{
  const adc_regs::Config c = {
    .COMP_QUE = 3,
    .COMP_LAT = 0,
    .COMP_POL = 0,
    .COMP_MODE = 0,
    .DR = 7,
    .MODE = 1,
    //.PGA = 2,
    .PGA = 0,
    .MUX = 4 | channel,
    .OS = 1,
  };
  //LOG("sizeof(adc_regs::Config)=%zu", sizeof(adc_regs::Config));
  //adc_regs::dump(c);

  const auto v = *(uint16_t*) &c;
  const auto r = write(Register::Config, v);
  return r;
}

} // namespace i2c
} // namespace nc
