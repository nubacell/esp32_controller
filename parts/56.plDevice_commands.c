
static const char * const plDevice_commands[] = {
  ":SYSTem:BAUD?",
  ":SYSTem:BEEP?",
  ":SYSTem:PORT?",
  //":SYSTem:DEVINFO?", // returns 7 lines
  ":STATus?",
  ":INPut?",
  ":VOLTage?",
  ":VOLTage:UPPer?",
  ":VOLTage:LOWer?",
  ":CURRent?",
  ":CURRent:UPPer?",
  ":CURRent:LOWer?",
  ":MEASure:CURRent?",
  ":MEASure:VOLTage?",
  ":MEASure:POWer?",
};
static void plDevice_commands_try()
{
  const char * CMD_DEVINFO = ":SYSTem:DEVINFO?";
  plDevice.command_try(CMD_DEVINFO, 7);
    ARRAY_SIZE(plDevice_commands);
#if 0
  plDevice.commands_try(
    plDevice_commands,
    ARRAY_SIZE(plDevice_commands));
#endif
}
