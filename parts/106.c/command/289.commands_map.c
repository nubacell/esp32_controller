
static const commands_map_t commands_map[] = {
#define CMD(x) { #x, CONSTSTRLEN(#x), x, }
  CMD(u1_hex),
  CMD(u2_hex),
  CMD(u1_cr),
  CMD(u2_cr),
  CMD(u1_crlf),
  CMD(u2_crlf),
  CMD(u1_lf),
  CMD(u2_lf),
  CMD(u1_lfcr),
  CMD(u2_lfcr),
  CMD(u1_baud),
  CMD(u2_baud),
#undef CMD
#define COMMAND(cmd,func) { cmd, CONSTSTRLEN(cmd), func, }
  COMMAND("*IDN?", command_idn),
#undef COMMAND
#define COMMAND(cmd) { #cmd, CONSTSTRLEN(#cmd), cmd, }

  COMMAND(el_id),
  COMMAND(el_clear_errors),
  //COMMAND(el_release_control),
  COMMAND(el_function),
  COMMAND(el_input),

  COMMAND(el_cc_range),
  COMMAND(el_cv_range),
  COMMAND(el_cc_level),
  COMMAND(el_cv_level),

  COMMAND(profile_steps),
  COMMAND(profile_steps_dump),
  COMMAND(profile_step),

  COMMAND(test_run),
  COMMAND(test_abort),
  COMMAND(test_dump),

  COMMAND(spi_run),
  COMMAND(spi_stats),

  COMMAND(ps_id),
  COMMAND(ps_v),
  COMMAND(ps_c),
  COMMAND(ps_ovp),
  COMMAND(ps_ocp),
  COMMAND(ps_output),

  COMMAND(scope_sync_on),
  COMMAND(scope_sync_off),
  COMMAND(scope_sync_pulse),

  COMMAND(pin_dump),
  COMMAND(pin_reset),
  COMMAND(pin_level),
  COMMAND(pin_mode),
  COMMAND(pin_pull),
#undef COMMAND
};

static
void
print_commands(
    const commands_map_t * commands_map,
    size_t commands_count)
{
  LOG("Available commands:");
  for (unsigned i = 0; i < commands_count; ++i) {
    LOG(". %s", commands_map[i].command_s);
  }
}

void print_commands()
{
  print_commands(commands_map, ARRAY_SIZE(commands_map));
}

static
command_fn_t
command_s_to_fn(
    const commands_map_t * commands_map,
    size_t commands_count,
    const char * command_s,
    size_t command_len)
{
  for (unsigned i = 0; i < commands_count; ++i) {
    if (commands_map[i].command_len == command_len
    && !strncmp(command_s, commands_map[i].command_s, command_len)) {
      return commands_map[i].command_fn;
    }
  }
  return NULL;
}

command_fn_t
command_s_to_fn(
    const char * command_s,
    size_t command_len)
{
  return command_s_to_fn(
      commands_map, ARRAY_SIZE(commands_map),
      command_s, command_len);
}
