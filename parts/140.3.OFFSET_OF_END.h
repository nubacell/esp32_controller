
#define OFFSET_OF_END(container_type, member_name) \
    (OFFSET_OF(container_type, member_name) \
        + SIZE_OF_MEMBER(container_type, member_name))
