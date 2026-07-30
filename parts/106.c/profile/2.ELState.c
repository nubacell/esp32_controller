
const char * to_string(ELState elstate) {
  switch (elstate) {
#define CASE_RETURN(x) case ELState::x: return #x;
    CASE_RETURN(off);
    CASE_RETURN(CC);
    CASE_RETURN(CV);
#undef CASE_RETURN
    default: return "?";
  }
}
