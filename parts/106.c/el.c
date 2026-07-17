
// TODO: Use 'enum class' instead of 'unsigned'

const char * const el_functions[] = { "CC", "CV", };

const char * el_function_to_string(unsigned function)
{
  return el_functions[function];
}


const char * const el_ranges[] = { "HIGH", "LOW", "MID", };

const char * el_range_to_string(unsigned cc_range)
{
  return el_ranges[cc_range];
}
