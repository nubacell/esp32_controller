
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
