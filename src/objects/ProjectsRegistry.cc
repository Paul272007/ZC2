#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
#include <objects/ProjectsRegistry.hh>
#include <objects/ZCError.hh>
#include <vector>

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

ProjectsRegistry &ProjectsRegistry::getInstance()
{
  static ProjectsRegistry instance;
  static bool initialized = false;
  if (!initialized)
  {
    instance.load();
    initialized = true;
  }
  return instance;
};

void ProjectsRegistry::load()
{
  json json_projects;
  if (!fs::exists(projects_path_))
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The projects registry was not found: " +
                                           projects_path_.string());
    return;
  }
  ifstream input(projects_path_);
  if (!input.is_open())
    throw ZCError(ZC_CONFIG_READING_ERROR,
                  "The projects registry couldn't be read: " +
                      projects_path_.string());
  try
  {
    input >> json_projects;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(ZC_CONFIG_PARSING_ERROR,
                  "The projects registry couldn't be parsed: " +
                      projects_path_.string() + ": " + e.what());
  }
  if (json_projects.contains("projects") &&
      json_projects["projects"].is_object())
  {
    for (auto &[key, value] : json_projects["projects"].items())
    {
      ProjectData p;
      p.name_ = key;
      p.path_ = value.get<std::string>();

      projects_.push_back(p);
    }
  }
};

bool ProjectsRegistry::saveProject(const ProjectData &p)
{
  projects_.push_back(p);
  json root;
  root["projects"] = json::object();

  for (const auto &p : projects_)
  {
    root["projects"][p.name_] = p.path_;
  }
  ofstream output(projects_path_);
  if (!output.is_open())
  {
    throw ZCError(ZC_CONFIG_WRITING_ERROR,
                  "The projects registry couldn't be written: " +
                      projects_path_.string());
    return false;
  }
  output << root.dump(4);
  output.close();
  return true;
};

Table ProjectsRegistry::projectsTable()
{
  vector<vector<string>> str_projects{{"Project name", "Path"}};

  for (const auto &p : projects_)
    str_projects.push_back({p.name_, p.path_.string()});

  return Table(str_projects.size() + 1, 2, false, true, str_projects);
};

vector<ProjectData> ProjectsRegistry::getProjects_() { return projects_; }
