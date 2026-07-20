
uint32_t cpu_sp_get()
{
  register uint32_t rsp asm("sp");
  return rsp;
}
