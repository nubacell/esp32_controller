
// Currently 'loop time' is in 'milliseconds' ('ms')

namespace nc {
namespace loop {

using time_t = uint32_t;

time_t now() { return ::millis(); }

} // namespace loop
} // namespace nc
