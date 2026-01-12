#ifndef HEADER_H_
#define HEADER_H_

#include <Command.hh>

class Header : public Command
{
public:
  Header();
  virtual int execute() override;
  virtual ~Header();
};

#endif // !HEADER_H_
