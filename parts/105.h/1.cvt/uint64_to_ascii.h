
namespace nc {
namespace mk {
namespace cvt {

// Assuming 'base' is 2, 8, 10, or 16.
size_t
uint64_to_ascii(
    char * buffer,
    size_t buffer_size,
    uint64_t number,
    uint8_t base,
    bool upperCase);

} // namespace cvt
} // namespace mk
} // namespace nc
