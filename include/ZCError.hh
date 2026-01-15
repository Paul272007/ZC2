#ifndef ZCERROR_H_
#define ZCERROR_H_

#include <colors.h>
#include <exception>
#include <iostream>
#include <string>

enum ErrorCode
{
  // Success
  SUCCESS = 0,
  // Program errors
  ZC_COMPILATION = 10,
  ZC_EXECUTION = 11,
  // File errors
  ZC_FILE_NOT_FOUND = 20,
  ZC_FILE_WRITING_ERROR = 21,
  ZC_FILE_PARSING_ERROR = 22,
  // Configuration errors
  ZC_CONFIG_PARSING_ERROR = 30,
  ZC_CONFIG_NOT_FOUND = 31,
  // Command errors
  ZC_BAD_COMMAND = 40,
  ZC_UNSUPPORTED_TYPE = 41,
  ZC_NO_MAIN_FUNCTION = 42,
  // Keyboard Interrupt / Operations aborted
  ZC_USER_INTERRUPT = 50,
  // Internal errors
  ZC_INTERNAL_ERROR = 60
};

class ZCError : public std::exception
{
public:
  ZCError(ErrorCode code, std::string message);
  void display(std::ostream &f = std::cout) const;
  int code() const;
  virtual ~ZCError();

private:
  ErrorCode code_;
  std::string message_;
};

std::ostream &operator<<(std::ostream &s, const ZCError &e);

#endif // !ZCERROR_H_
