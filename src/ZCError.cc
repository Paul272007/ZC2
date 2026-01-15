#include <ZCError.hh>

using namespace std;

ZCError::ZCError(ErrorCode code, string message)
    : code_(code), message_(message)
{
}

ZCError::~ZCError() {}

void ZCError::display(ostream &f) const
{
  f << RED << "[ERROR] " << COLOR_RESET << "(exit code: " << code_ << ") "
    << message_;
}

ostream &operator<<(ostream &s, const ZCError &e)
{
  e.display(s);
  return s;
}

int ZCError::code() const { return (int)code_; }
