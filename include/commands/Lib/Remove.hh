#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/Registry.hh>

class Remove : public Command
{
public:
  /**
   * @brief Uninstall given libraries
   *
   * @param targets The libraries to be installed
   */
  Remove(const std::vector<std::string> &targets);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  Registry &registry_;
  const std::vector<std::string> targets_;
};
