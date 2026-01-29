#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/ProjectsRegistry.hh>
#include <objects/Settings.hh>
#include <vector>

#define PROJECT_TEMPLATES "project_templates"

class Project : public Command
{
public:
  /**
   * @brief Create a new project in a new folder using ZC
   *
   * @param language The language of the new project (C or C++)
   * @param project_name The name of the new project / folder
   * @param force Whether to force creating project even if it already exists
   * @param edit Whether to edit the project once created
   */
  Project(const std::string &language, const std::string &project_name,
          bool force, bool edit, bool git);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  virtual int execute() override;

private:
  /**
   * @brief Fetch all templates from the project templates folder
   */
  std::vector<std::string> getProjectTemplates();

  std::vector<std::string> project_templates_;
  const std::string language_;
  const std::string project_name_;
  const std::filesystem::path project_path_;
  const bool force_;
  const bool edit_;
  const bool git_;
  Settings &settings_;
  ProjectsRegistry &pregistry_;
  std::filesystem::path project_templates_path_ =
      getZCRootDir() / PROJECT_TEMPLATES;
};
