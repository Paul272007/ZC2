#include <filesystem>
#include <string>
#include <vector>

#include <commands/Project.hh>
#include <helpers.hh>
#include <objects/ProjectsRegistry.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;
namespace fs = std::filesystem;

Project::Project(const string &language, const string &project_name, bool force,
                 bool edit, bool git)
    : language_(language), project_name_(split(project_name, '/').back()),
      project_path_(project_name), force_(force), edit_(edit), git_(git),
      settings_(Settings::getInstance()),
      pregistry_(ProjectsRegistry::getInstance())
{
}

vector<string> Project::getProjectTemplates()
{
  vector<string> templates_list;
  try
  {
    if (fs::exists(project_templates_path_) &&
        fs::is_directory(project_templates_path_))
      for (const auto &entry : fs::directory_iterator(project_templates_path_))
        if (entry.is_directory())
          templates_list.push_back(entry.path().string());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return templates_list;
}

int Project::execute()
{
  if (!force_ && fs::exists(project_path_) &&
      pregistry_.projectExists(project_name_))
    if (!ask("The directory and/or project " + project_path_.string() +
             " already exists. Do you want to replace it ?"))
      return 0;

  vector<string> templates_list = getProjectTemplates();
  bool found = false;

  for (const auto &dir : templates_list)
  {
    if (split(dir, '.').back() == language_)
    {
      fs::copy(dir, project_path_,
               fs::copy_options::recursive |
                   fs::copy_options::overwrite_existing);
      found = true;
      break;
    }
  }

  if (!found)
    throw ZCError(ZC_UNSUPPORTED_LANGUAGE,
                  "No template is available for the language: " + language_);

  if (git_)
  {
    info("Initializing git repo...");
    if (system(string("git init " + project_path_.string()).c_str()) != 0)
      throw ZCError(ZC_GIT_ERROR, "Git init failed");
  }

  ProjectData p{project_name_, project_path_};
  pregistry_.saveProject(p);

  if (settings_.edit_on_init_ || edit_)
    return system(
        string(settings_.editor_ + " " + project_path_.string()).c_str());

  return 0;
}
