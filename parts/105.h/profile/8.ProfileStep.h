
struct ProfileStep
{
  uint32_t step_ms;
  uint32_t sub_step_ms;
   int32_t I_A;
  uint32_t PS_V_max;
  uint32_t cBat_max_mV;
  uint32_t cBat_min_mV;
  uint32_t aBat_max_mV;
  uint32_t aBat_min_mV;
  uint32_t cBat_T_max_C;
  uint32_t aBat_T_max_C;

  void dump() const;
  void apply(const char * args);
};

constexpr uint32_t profile_steps_count_max = 100;
static ProfileStep profileSteps_[profile_steps_count_max];
