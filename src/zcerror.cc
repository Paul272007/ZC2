#include <zcerror.hpp>

using namespace std;

ZCError::ZCError(int code, string message) : m_code(code), m_message(message) {}

ZCError::~ZCError() {}

void ZCError::display(ostream &f) const
{
  f << RED << "[ERROR] " << COLOR_RESET << "(exit code: " << m_code << ") "
    << m_message;
}

ostream &operator<<(ostream &f, const ZCError &e)
{
  e.display(f);
  return f;
}

int ZCError::code() const { return m_code; }
