
__attribute__((noinline))
void stack_marker_fill(uint32_t * addr, size_t size, uint8_t fill);
#define STACK_MARKER(size,fill) \
    { \
      uint32_t stack_marker[size/sizeof(uint32_t)]; \
      stack_marker_fill(stack_marker, sizeof(stack_marker), fill); \
    }
