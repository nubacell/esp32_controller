
static void log(const char *file, unsigned line, const char * function, const char * format, ...);
#define LOG(format, args...) log(__FILE__, __LINE__, __FUNCTION__, format, ##args)
