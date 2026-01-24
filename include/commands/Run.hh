#pragma once

#include "objects/Registry.hh"
#include "objects/Settings.hh"
#include <objects/File.hh>
#include <string>
#include <vector>

#include <commands/Command.hh>

enum Mode
{
  FULL,
  PREPROCESS,
  COMPILE,
  ASSEMBLE
};

class Run : public Command
{
public:
  Run(const std::vector<std::string> &files,
      const std::vector<std::string> &args, bool keep, bool plus,
      bool preprocess, bool compile, bool assemble);

  int execute() override;

private:
  Mode getMode(bool preprocess, bool compile, bool assemble) const;

  bool isCppAndCheckExtensions(std::string &badFile) const;

  std::string buildCommand(const std::string &output_name) const;

  bool filesExist(std::string &badFile) const;

  std::vector<std::string> getInclusions() const;

  bool keep_ = false;

  bool plus_ = false;

  Mode mode_ = FULL;

  Settings &settings_;

  Registry &registry_;

  std::vector<std::string> args_;

  std::vector<File> files_;
};
