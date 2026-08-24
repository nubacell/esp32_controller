
void Test::on_state_cleanup(uint32_t now_ms)
{
  // --- 'cleanup' ---
  deactivate_PS_EL();
  LOG("--- test finished ---");
  Serial.print("/TEST/FINISHED\n");
  ::rpi::reportStats();
  state(State::idle);
}
