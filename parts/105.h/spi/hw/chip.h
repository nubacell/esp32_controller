
namespace nc {
namespace spi {
namespace hw {

struct Chip
{
  bool active_ = false;
  bool init();
  void down();
};

} // namespace hw
} // namespace spi
} // namespace nc
