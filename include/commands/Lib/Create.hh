#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>

class Create : public Command
{
public:
  Create(const std::string &package_name, const std::vector<std::string> &files,
         bool force);
  virtual int execute() override;

private:
  bool isShared() const;

  bool force_;
  Registry &registry_;
  std::string package_name_;
  std::vector<File> files_;
};
