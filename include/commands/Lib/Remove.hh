#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/Registry.hh>

class Remove : public Command
{
public:
  Remove(const std::vector<std::string> &targets);
  virtual int execute() override;

private:
  Registry &registry_;
  const std::vector<std::string> targets_;
};
