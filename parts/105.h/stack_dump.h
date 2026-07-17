
//__attribute__((noinline))
__attribute__((noipa))
void stack_recurse(unsigned depth);

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump(unsigned line, const char * func);
#define STACK_DUMP() stack_dump(__LINE__, __FUNCTION__)

//__attribute__((noinline))
__attribute__((noipa))
void stack_dump_recursive(unsigned line, const char * func, unsigned depth);
#define STACK_DUMP_RECURSIVE(depth) stack_dump_recursive(__LINE__, __FUNCTION__, depth)
