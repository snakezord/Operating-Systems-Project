#if !defined(_LOGS_H_)
#define _LOGS_H_

int init_logs();
void log(const char*string);
void log_int(const char*string, int n);
void log_str(const char*str, char*s);
void log_close();

#endif