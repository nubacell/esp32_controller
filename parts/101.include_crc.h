
// https://github.com/RobTillaart/CRC
#include "CRC32.h"

/*
  The following function declaration shall be automatically
  taken from #include "CRC32.h" above but build fails if it
  is not repeated here.
*/
uint8_t calcCRC8(
    const uint8_t *array, crc_size_t length,
    uint8_t polynome   = CRC8_POLYNOME,
    uint8_t initial    = CRC8_INITIAL,
    uint8_t xorOut     = CRC8_XOR_OUT,
    bool reverseIn     = CRC8_REV_IN,
    bool reverseOut    = CRC8_REV_OUT,
    crc_size_t yieldPeriod = CRC_YIELD_DISABLED);
