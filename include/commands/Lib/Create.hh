#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>

class Create : public Command
{
public:
  /**
   * @brief Create a new library
   *
   * @param package_name The name of the new package
   * @param files The files used to create the new library
   * @param force Whether to force creating the library even if it already
   * exists
   */
  Create(const std::string &package_name, const std::vector<std::string> &files,
         bool force);

  /**
   * @brief Execute the command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  bool force_;
  Registry &registry_;
  std::string package_name_;
  std::vector<File> files_;
};
