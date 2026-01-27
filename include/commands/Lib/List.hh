#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  /**
   * @brief Display installed libraries
   */
  List(bool display_std);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  const bool display_std_ = false;
  const Registry &registry_;
};
