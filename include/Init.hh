#ifndef HEADER_H_
#define HEADER_H_

#include <Command.hh>

class Init : public Command
{
public:
  Init();
  virtual int execute() override;
  virtual ~Init();
};

#endif // !HEADER_H_
