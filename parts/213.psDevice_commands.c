
// "TENMA 72-2710 V6.6 SN:31428462"
/*
  settings:

    ISET<X>:<NR2>
    VSET<X>:<NR2>
    ISET<X>?
    VSET<X>?

  actual output

    IOUT<X>?
    VOUT<X>?

  output on/off

    OUT<Boolean>

  STATUS?
*/
static const char * const psDevice_commands[] = {
  "ISET1?",
  "VSET1?",
  "IOUT1?",
  "VOUT1?",
  "STATUS?",
};
static void psDevice_commands_try()
{
  psDevice.commands_try(
    psDevice_commands,
    ARRAY_SIZE(psDevice_commands));
}
