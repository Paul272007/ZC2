#pragma once

#include <string>

#include <commands/Command.hh>
#include <objects/Registry.hh>

class Install : public Command
{
public:
  Install(const std::string &target);
  virtual int execute() override;

private:
  const std::string target_;

  Registry &registry_;
};
