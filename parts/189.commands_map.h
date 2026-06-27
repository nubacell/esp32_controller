
typedef void (*command_fn_t)(const char * args);

typedef struct commands_map_s {
  const char * command_s;
  size_t command_len;
  command_fn_t command_fn;
} commands_map_t;

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
