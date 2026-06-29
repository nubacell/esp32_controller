
struct Test
  : Runnable
{
  unsigned step_;
  unsigned sub_step_;
  //uint32_t start_time_ms_;
  uint32_t event_time_ms_;

  bool run_;
  bool abort_;

  enum class State {
    init,
    idle,
    running,
  };
  static const char * to_string(State state);

  void state(State new_state)
  {
    const auto old_state_name = to_string(state_);
    state_ = new_state;
    const auto new_state_name = to_string(state_);
    LOG("'%s' -> '%s'", old_state_name, new_state_name);
  }

  State state_ = State::init;

  void on_state_init(uint32_t now_ms)
  {
    state(State::idle);
  }


  void change_PS_EL_configs(const ProfileStep * profileStep)
  {
    if (ELState::off == profileStep->elconfig_.state_) {
      LOG("deactivatin EL");
      elDevice.set_input_OFF();
    } else if (ELState::CC == profileStep->elconfig_.state_) {
      LOG("changing EL config");
      elDevice.cc_range(profileStep->elconfig_.range_);
      elDevice.cc_level(profileStep->elconfig_.level_);
      elDevice.cc_level();
      elDevice.set_input_ON();
    } else {
      LOG("ERROR: Unexpected EL state");
    }
  }

  void deactivate_PS_EL()
  {
    LOG("deactivating EL");
    elDevice.set_input_OFF();
  }


  void on_state_idle(uint32_t now_ms)
  {
    if (run_) {
      run_ = false;
      abort_ = false;

      if (!profile.steps_count_) {
        LOG("ERROR: profile is empty");
        state(State::idle);
      } else {
        state(State::running);

        step_ = 0;
        sub_step_ = 0;

        const ProfileStep * profileStep = &profileSteps_[step_];

        //start_time_ms_ = now_ms;
        event_time_ms_ = now_ms;

        // Change PS and EL configs.
        change_PS_EL_configs(profileStep);

        // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
        // be executed on next 'on_loop'.
        if (profileStep->substeps_count_
        && profileStep->measure_period_ms_) {
          event_time_ms_ += profileStep->measure_period_ms_;
        }
      }
    }
  }

  void on_event(uint32_t now_ms)
  {
    LOG("--- sub_step_=%u finished -- do measurements...", sub_step_);

    const ProfileStep * profileStep = &profileSteps_[step_];
    ++sub_step_;
    if (sub_step_ < profileStep->substeps_count_) {
      event_time_ms_ += profileStep->measure_period_ms_;
    } else {
      LOG("--- step_=%u finished -- change PS and EL configs...", step_);

      ++step_;
      if (step_ < profile.steps_count_) {

        profileStep = &profileSteps_[step_];
        sub_step_ = 0;

        // Change PS and EL configs.
        change_PS_EL_configs(profileStep);

        // if !substeps_count_ || measure_period_ms_ then next 'on_event' will
        // be executed on next 'on_loop'.
        if (profileStep->substeps_count_
        && profileStep->measure_period_ms_) {
          event_time_ms_ += profileStep->measure_period_ms_;
        }

      } else {
        deactivate_PS_EL();
        LOG("--- test finished ---");
        // 'test' finished
        state(State::idle);
      }
    }
  }

  void on_state_running(uint32_t now_ms)
  {
    if (abort_) {
      abort_ = false;

      deactivate_PS_EL();

      state(State::idle);
    } else
    if (event_time_ms_ <= now_ms) {
      on_event(now_ms);
    }
  }

  void on_loop(uint32_t now_ms) override
  {
    switch (state_) {
#define CASE_CALL(x) case State::x: on_state_##x(now_ms); break;
      CASE_CALL(init)
      CASE_CALL(idle)
      CASE_CALL(running)
#undef CASE_CALL
      default:
        LOG("Unexpected state=%u/'%s'", state_, to_string(state_));
        break;
    }
  }

  void run()
  {
    run_ = true;
  }

  void abort()
  {
    abort_ = true;
  }
};
