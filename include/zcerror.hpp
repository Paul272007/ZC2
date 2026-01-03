#ifndef ZCERROR_H_
#define ZCERROR_H_

#include <colors.h>
#include <exception>
#include <iostream>
#include <string>

class ZCError : public std::exception {
public:
  ZCError(int code, std::string message);
  void display(std::ostream &f = std::cout) const;
  virtual ~ZCError();

private:
  int m_code;
  std::string m_message;
};

std::ostream &operator<<(std::ostream &s, const ZCError &e);

#endif // !ZCERROR_H_
