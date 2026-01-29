#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <objects/File.hh>
#include <zcio.hh>

#define PROJECTS "projects.json"

struct ProjectData
{
  std::string name_;
  std::filesystem::path path_;
  Language language_;
};

class ProjectsRegistry
{
public:
  static ProjectsRegistry &getInstance();

  Table projectsTable();

  bool saveProject(const ProjectData &p);

  bool projectExists(const std::string &target);

private:
  ProjectsRegistry();

  void load();

  std::vector<ProjectData> getProjects_();

  std::vector<ProjectData> projects_;

  std::filesystem::path projects_path_ = getZCRootDir() / PROJECTS;
};
