
uint32_t regs_[8];
//__attribute__((noinline))
__attribute__((noipa))
void save_regs(
    uint32_t r0,
    uint32_t r1,
    uint32_t r2,
    uint32_t r3,
    uint32_t r4,
    uint32_t r5)
{
  regs_[ 0] = r0;
  regs_[ 1] = r1;
  regs_[ 2] = r2;
  regs_[ 3] = r3;
  regs_[ 4] = r4;
  regs_[ 5] = r5;
}

unsigned stack_recurse_count_;
//__attribute__((noinline))
__attribute__((noipa))
void stack_recurse(unsigned depth)
{
  asm volatile ("");
  STACK_MARKER(16, '1');
  if (depth) {
    stack_recurse(depth - 1);
  }
#if 1
  {
    save_regs(
        cpu_sp_get() & 0xFFFFFF00,
        1,
        2,
        3,
        4,
        5);
  }
  ++stack_recurse_count_;
#endif
}

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump_recursive(unsigned line, const char * func, unsigned depth)
{
  asm volatile ("");
  STACK_MARKER(16, '2');
  stack_recurse(4);
  //const void * p = &p;
  //LOG("_ %u:%s p=%p", line, func, p);
  //LOG("_ %u:%s sp=%p p=%p", line, func, cpu_sp_get(), p);
  LOG("_ %u:%s sp=%p", line, func, cpu_sp_get());
  if (depth) {
    stack_dump_recursive(line, func, depth - 1);
  } else {
    stack_dump(line, func);
  }
  ++stack_recurse_count_;
}

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump(unsigned line, const char * func)
{
  asm volatile ("");
  STACK_MARKER(16, '3');
  //stack_recurse(4);
  //const void * p = &p;
  //LOG("_ %u:%s p=%p", line, func, p);
  //LOG("_ %u:%s sp=%p p=%p", line, func, cpu_sp_get(), p);
  LOG("_ %u:%s sp=%p", line, func, cpu_sp_get());
  //hex_dump((const uint8_t*)p, 256);
  //hex_dump( ((const uint8_t*)p) - 32, 256);
  hex_dump( ((const uint8_t*)cpu_sp_get()) - 256, 256*3);
}
