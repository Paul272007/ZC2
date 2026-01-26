#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  /**
   * @brief Display installed libraries
   */
  List();

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  const Registry &registry_;
};
