
// Note: We age going to use "EL" in "CC" mode only

enum class ELState {
  off,
  CC,
  CV,
};
const char * to_string(ELState elstate);
