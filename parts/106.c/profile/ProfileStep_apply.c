
void ProfileStep::apply(const char * args)
{
  const char * tail = args;

  if (!tail || !*tail) { return; }
  step_ms = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  I_A = command_args_to_i32(tail, &tail);

  if (!tail || !*tail) { return; }
  sub_step_ms = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  PS_V_max = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_max_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_min_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_max_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_min_mV = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  cBat_T_max_C = command_args_to_u32(tail, &tail);

  if (!tail || !*tail) { return; }
  aBat_T_max_C = command_args_to_u32(tail, &tail);
}
