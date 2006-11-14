//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

void check_condition(const char *p_file_name, unsigned int src_line, const char* desc);
void check_condition_val(const char *p_file_name, unsigned int src_line, const char* desc, const int err_val);

#define CHECK_CONDITION(expr, desc) \
if (expr) \
; \
else \
check_condition(__FILE__, __LINE__, Desc)

#define CHECK_CONDITION_VAL(Expr, Desc, Err_val) \
if (Expr) \
; \
else \
check_condition_val(__FILE__, __LINE__, Desc, Err_val)


int logger_init(const char* appname);
void logger_shutdown();

//
// This should be used if an assertion fails.
void log_crit(const char *message);
void log_crit(const char *message, int error_val);


//
// non critical failure: ie log file is bad
void log_error(const char *message);
void log_error(const char *message, int error_val);

//
// Information that might interest user, but isn't a problem
void log_info(const char *message);
void log_info(const char *message, int error_val);
void log_info(const char *message1, const char *message2);

//
// Information that help us debug internally.  Will not
// display in release mode.
void log_debug(const char *message);
void log_debug(const char *message, int val);


#endif
