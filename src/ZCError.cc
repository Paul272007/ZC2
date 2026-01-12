#include <ZCError.hh>

using namespace std;

ZCError::ZCError(int code, string message) : m_code(code), m_message(message) {}

ZCError::~ZCError() {}

void ZCError::display(ostream &f) const
{
  f << RED << "[ERROR] " << COLOR_RESET << "(exit code: " << m_code << ") "
    << m_message;
}

ostream &operator<<(ostream &s, const ZCError &e)
{
  e.display(s);
  return s;
}

int ZCError::code() const { return m_code; }
