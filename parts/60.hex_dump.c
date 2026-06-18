
void hex_dump(const uint8_t * data, size_t size)
{
  unsigned i;
  for (i = 0; i < size; ++i) {
    if (0 == (i % 16)) {
      if (i) {
        Serial.println("");
      }
      Serial.print(". . ");
      Serial.print(i/16, HEX);
      Serial.print(":");
    } else if (0 == (i % 4)) {
      Serial.print(" ");
    }
    Serial.print(" ");
    const uint8_t b = data[i];
    Serial.print((b >> 4) & 0x0F, HEX);
    Serial.print( b       & 0x0F, HEX);
  }
  if (i) {
    Serial.println("");
  }
}
