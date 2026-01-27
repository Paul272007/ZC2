#pragma once

#include <exception>
#include <ostream>
#include <string>

enum ErrorCode
{
  // No errors
  SUCCESS = 0,
  // Program errors
  ZC_COMPILATION_ERROR = 10,
  ZC_EXECUTION_ERROR = 11,
  // File errors
  ZC_NOT_FOUND = 20,
  ZC_WRITING_ERROR = 21,
  ZC_PARSING_ERROR = 22,
  // Configuration errors
  ZC_CONFIG_PARSING_ERROR = 30,
  ZC_CONFIG_NOT_FOUND = 31,
  ZC_CONFIG_READING_ERROR = 32,
  ZC_CONFIG_WRITING_ERROR = 33,
  ZC_CONFIG_CONTENT_ERROR = 34,
  // Command errors
  ZC_BAD_COMMAND = 40,
  ZC_UNSUPPORTED_LANGUAGE = 41,
  ZC_INCOMPATIBLE_FLAGS = 42,
  // User errors
  ZC_KEYBOARD_INTERRUPT = 50,
  ZC_OPERATIONS_ABORTED = 51,
  // Internal errors
  ZC_INTERNAL_ERROR = 60,
  // Package errors
  ZC_PACKAGE_NOT_FOUND = 70
};

class ZCError : public std::exception
{
public:
  /**
   * @brief Default constructor just to not get errors when throwing empty
   * errors
   */
  ZCError() = default;

  /**
   * @brief Create ZCError instance
   *
   * @param code The code corresponding to the error type
   * @param message A message that explains the error
   */
  ZCError(ErrorCode code, const std::string &message);

  /**
   * @brief Display error to stream
   *
   * @param stream The stream in which the error is displayed
   */
  void display(std::ostream &stream) const;

  /**
   * @brief Get the error code as an int
   *
   * @return code_ as an int
   */
  int getCode_() const;

private:
  ErrorCode code_ = SUCCESS;
  std::string message_ = "Success";
};

/**
 * @brief << overload for ZCError
 *
 * @param stream The stream in which the error is written
 * @param zcerror The error to be displayed
 */
std::ostream &operator<<(std::ostream &stream, const ZCError &zcerror);
