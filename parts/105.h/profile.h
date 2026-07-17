
enum class PSState {
  off,
  CC,
  CV,
};
const char * to_string(PSState psstate);

struct PSConfig {
  PSState state_;
  double i_;
  double v_;

  void dump() const {
    LOG(". . %s state='%s' I=%.3f V=%.3f", "PS", to_string(state_), i_, v_);
  }
};

// Note: We age going to use "EL" in "CC" mode only

enum class ELState {
  off,
  CC,
  CV,
};
const char * to_string(ELState elstate);

struct ELConfig {
  ELState state_;
  uint8_t range_;
  double level_;

  void dump() const {
    LOG(". . %s state_=%u/'%s' range=%u/'%s' level=%.6f", "EL",
        state_, to_string(state_), range_, el_range_to_string(range_), level_);
  }

  void apply(const char * args)
  {
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      state_ = (ELState) strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      range_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      level_ = strtod(args, &endptr);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
  }
};

struct ProfileStep {
  uint32_t substeps_count_ = 0;
  uint32_t measure_period_ms_;
  //PSConfig psconfig_;
  ELConfig elconfig_;

  void dump() const {
    LOG(". this=%p substeps_count=%u measure_period_ms=%u",
        this, substeps_count_, measure_period_ms_);
    //psconfig_.dump();
    elconfig_.dump();
  }

  void apply(const char * args)
  {
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      substeps_count_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    {
      args = skip_whitespace(args);
      char * endptr = NULL;
      measure_period_ms_ = strtoul(args, &endptr, 0);
      if (!endptr || !*endptr) { return; }
      args = endptr;
    }
    elconfig_.apply(args);
  }
};

constexpr uint32_t profile_steps_count_max = 100;
static ProfileStep profileSteps_[profile_steps_count_max];

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
