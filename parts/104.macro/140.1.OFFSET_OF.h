
#define OFFSET_OF(container_type, member_name) \
    ((size_t)&(((container_type *)0)->member_name))
