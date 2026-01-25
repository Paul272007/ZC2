#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/Settings.hh>
#include <vector>

#define PROJECT_TEMPLATES "project_templates"

class Project : public Command
{
public:
  Project(const std::string &language, const std::string &project_name,
          bool force, bool edit);
  virtual int execute() override;

private:
  std::vector<std::string> getProjectTemplates();

  std::vector<std::string> project_templates_;
  const std::string language_;
  const std::string project_name_;
  const bool force_;
  const bool edit_;
  Settings &settings_;
  std::filesystem::path project_templates_path_ =
      getZCRootDir() / PROJECT_TEMPLATES;
};
