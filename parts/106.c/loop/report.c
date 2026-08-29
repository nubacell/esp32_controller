
namespace nc {
namespace loop {

void Stats::report(uint32_t now_ms) const
{
  Serial.print(device_tag);
  Serial.print(" now_ms="); Serial.print(now_ms);
  Serial.print(" loop_count="); Serial.println(loop_count_);
}

} // namespace loop
} // namespace nc
