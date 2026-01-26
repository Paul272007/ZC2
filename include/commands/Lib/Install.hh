#pragma once

#include <string>

#include <commands/Command.hh>
#include <objects/Registry.hh>

class Install : public Command
{
public:
  /**
   * @brief Install a library from server (not implemented)
   *
   * @param target The name of the package to be installed
   */
  Install(const std::string &target);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  const std::string target_;

  Registry &registry_;
};
