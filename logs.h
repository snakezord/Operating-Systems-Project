//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#if !defined(_LOGS_H_)
#define _LOGS_H_

int init_logs();
void logger(const char*string);
void log_int(const char*string, int n);
void log_str(const char*str, char*s);
void log_close();

#endif