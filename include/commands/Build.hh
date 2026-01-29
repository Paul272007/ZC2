#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/Settings.hh>

class Build : public Command
{
public:
  Build(bool force, bool release_mode);
  virtual int execute() override;

private:
  bool generateCMakeLists(const std::vector<File> &sources,
                          const std::vector<std::string> &libs);

  std::vector<File> scanSources(const std::filesystem::path &root) const;
  std::vector<std::string>
  detectLibraries(const std::vector<File> &sources) const;

  bool force_;
  bool release_mode_;
  Registry &registry_;
  Settings &settings_;
};
