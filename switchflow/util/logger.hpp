#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

void check_condition(const char *pFileName, unsigned int SrcLine, const char* Desc);
void check_condition_val(const char *pFileName, unsigned int SrcLine, const char* Desc, const int ErrVal);

#define CHECK_CONDITION(Expr, Desc) \
if (Expr) \
; \
else \
check_condition(__FILE__, __LINE__, Desc)

#define CHECK_CONDITION_VAL(Expr, Desc, ErrVal) \
if (Expr) \
; \
else \
check_condition_val(__FILE__, __LINE__, Desc, ErrVal)


int logger_init(const char* appname);
void logger_shutdown();

//
// This should be used if an assertion fails.
void log_crit(const char *message);
void log_crit(const char *message, int errorVal);


//
// non critical failure: ie log file is bad
void log_error(const char *message);
void log_error(const char *message, int errorVal);

//
// Information that might interest user, but isn't a problem
void log_info(const char *message);
void log_info(const char *message, int errorVal);
void log_info(const char *message1, const char *message2);

//
// Information that help us debug internally.  Will not
// display in release mode.
void log_debug(const char *message);
void log_debug(const char *message, int val);


#endif
