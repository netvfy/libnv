#ifndef LOG_H
#define LOG_H

void	log_setcb(void (*cb)(const char *));
void	log_info(const char *format, ...);
void	log_warnx(const char *format, ...);
void	log_warn(const char *format, ...);

#endif
