#ifndef COMMAND_H_
#define COMMAND_H_

class Command
{
public:
  virtual ~Command() = default;
  virtual int execute() = 0;
};

#endif // !COMMAND_H_
