
struct PSConfig {
  PSState state_;
  double i_;
  double v_;

  void dump() const {
    LOG(". . %s state='%s' I=%.3f V=%.3f", "PS", to_string(state_), i_, v_);
  }
};
