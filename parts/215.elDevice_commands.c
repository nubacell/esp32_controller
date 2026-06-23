
static const char * const elDevice_commands[] = {
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
static void elDevice_commands_try()
{
  const char * CMD_DEVINFO = ":SYSTem:DEVINFO?";
  elDevice.command_try(CMD_DEVINFO, 7);
    ARRAY_SIZE(elDevice_commands);
#if 0
  elDevice.commands_try(
    elDevice_commands,
    ARRAY_SIZE(elDevice_commands));
#endif
}
