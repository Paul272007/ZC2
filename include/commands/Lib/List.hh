#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  List();
  virtual int execute() override;

private:
  const Registry &registry_;
};
