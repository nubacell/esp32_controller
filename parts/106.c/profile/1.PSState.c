
const char * to_string(PSState psstate) {
  switch (psstate) {
#define CASE_RETURN(x) case PSState::x: return #x;
    CASE_RETURN(off);
    CASE_RETURN(CC);
    CASE_RETURN(CV);
#undef CASE_RETURN
    default: return "?";
  }
}
