
// abort test execution.
// Reached on user's request or on critical condition detection.

void Test::on_state_abort(uint32_t now_ms)
{
  abort_ = false;
  LOG("test execution aborted");
  state(State::cleanup);
}
