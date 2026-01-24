#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;

ZCError::ZCError(ErrorCode code, const string &message)
    : code_(code), message_(message)
{
}

void ZCError::display(ostream &stream) const
{
  stream << RED << "[ERROR] " << COLOR_RESET << "(exit code: " << code_ << ") "
         << message_;
}

ostream &operator<<(ostream &s, const ZCError &e)
{
  e.display(s);
  return s;
}

int ZCError::getCode_() const { return (int)code_; }
