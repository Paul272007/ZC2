#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <helpers.hh>

#define PROJECT_TEMPLATES "project_templates"

class Project : public Command
{
public:
  Project(const std::string &language, const std::string &project_name);
  virtual int execute() override;

private:
  const std::string language_;
  const std::string project_name_;
  std::filesystem::path project_templates_ = getZCRootDir() / PROJECT_TEMPLATES;
};
