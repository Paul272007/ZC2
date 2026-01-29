#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <zcio.hh>

#define PROJECTS "projects.json"

struct ProjectData
{
  std::string name_;
  std::filesystem::path path_;
};

class ProjectsRegistry
{
public:
  static ProjectsRegistry &getInstance();

  Table projectsTable();

  bool saveProject(const ProjectData &p);

private:
  ProjectsRegistry() = default;

  void load();

  std::vector<ProjectData> getProjects_();

  std::vector<ProjectData> projects_;

  std::filesystem::path projects_path_ = getZCRootDir() / PROJECTS;
};
