
typedef void (*command_fn_t)(const char * args);

typedef struct commands_map_s {
  const char * command_s;
  size_t command_len;
  command_fn_t command_fn;
} commands_map_t;

//extern
//const commands_map_t commands_map[];

void print_commands();

command_fn_t
command_s_to_fn(
    const char * command_s,
    size_t command_len);
