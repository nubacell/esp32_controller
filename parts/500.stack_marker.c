
__attribute__((noinline))
void stack_marker_fill(uint32_t * addr, size_t size, uint8_t fill)
{
  memset(addr, fill, size);
  if (size >= sizeof(uint32_t)) {
    *addr = uint32_t(addr);
  }
}
