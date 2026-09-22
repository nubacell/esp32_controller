
namespace nc {

static float V_to_I(float V)
{
  return V;
}

static float scale_I(uint16_t v)
{
  return V_to_I(float(v) * 0.0001875);
}

static float scale_V(int16_t v)
{
  return float(v) / 1000.;
}

static float V_to_T(float V)
{
  return (V - 0.4) / 0.019;
}

static float scale_T(uint16_t v)
{
  return V_to_T(float(v) / 1000.);
}

static void report_I_raw()
{
  Serial.print(" I0raw:");
  Serial.print(iMeters.iMeters_[0].value);

  Serial.print(" I1raw:");
  Serial.print(iMeters.iMeters_[1].value);
}

static void report_I()
{
  Serial.print(" I0:");
  Serial.print(scale_I(iMeters.iMeters_[0].value),7);

  Serial.print(" I1:");
  Serial.print(scale_I(iMeters.iMeters_[1].value),7);
}

static void report_V(
    unsigned spi_sensor_n,
    unsigned index)
{
  const auto & deviceData = ::nc::spi::sensors.sensors_[spi_sensor_n].raw.data;
  const auto & analogSignals = deviceData.analogSignals;
  const auto & voltages = analogSignals.voltages;
  Serial.print(" V");
  Serial.print(spi_sensor_n + 1);
  Serial.print(index);
  Serial.print(":");
  Serial.print(scale_V(voltages[index]),3);
}

static void report_T(
    unsigned spi_sensor_n,
    unsigned group,
    unsigned index)
{
  const auto & deviceData = ::nc::spi::sensors.sensors_[spi_sensor_n].raw.data;
  const auto & analogSignals = deviceData.analogSignals;
  const auto & temperatures = analogSignals.temperatures;
  Serial.print(" T");
  Serial.print(spi_sensor_n + 1);
  Serial.print(index);
  Serial.print(group);
  Serial.print(":");
  Serial.print(scale_T(temperatures[group][index]));
}

void Monitor::report_VT(unsigned spi_sensor_n) const
{
  if (!disable_.V_) {
    for (unsigned i = 0; i < ::nc::lgt::voltage_pins_COUNT; ++i) {
      report_V(spi_sensor_n, i);
    }
  }

  if (!disable_.T_) {
    for (unsigned i = 0; i < ::nc::lgt::temperature_pins_COUNT; ++i) {
      report_T(spi_sensor_n, 0, i);
      report_T(spi_sensor_n, 1, i);
    }
  }
}

void Monitor::report() const
{
  if (!disable_.I_) {
    if (!disable_.raw_) {
      report_I_raw();
    }
    report_I();
  }

  ;
  for (unsigned sensor_n = 0; sensor_n < SPI_SENSORS_COUNT; ++sensor_n) {
    if (!::nc::spi::sensors.offline(sensor_n)) {
      report_VT(sensor_n);
    }
  }

  Serial.println("");
}

} // namespace nc
