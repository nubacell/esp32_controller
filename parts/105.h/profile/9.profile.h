
struct Profile {
  uint32_t steps_count_ = 0;

  void dump() const {
    LOG("Profile: steps_count=%u", steps_count_);
  }

  void dump_steps() const {
    for (uint32_t i = 0; i < steps_count_; ++i) {
      ProfileStep * step = &profileSteps_[i];
      step->dump();
    }
  }

  void steps_count(uint32_t new_steps_count) {
    if (new_steps_count > profile_steps_count_max) {
      LOG("new_steps_count=%u > profile_steps_count_max=%u", new_steps_count, profile_steps_count_max);
    } else {
      steps_count_ = new_steps_count;
    }
  }
};

Profile profile;
