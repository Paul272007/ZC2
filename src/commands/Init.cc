#include <filesystem>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <commands/Init.hh>
#include <objects/File.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;
namespace fs = std::filesystem;

Init::Init(const vector<string> &files, bool force, vector<string> &input_files,
           bool edit)
    : force_(force), edit_(edit), settings_(Settings::getInstance())
{
  for (const auto &f : files)
    files_.push_back(File(f));
  for (const auto &i : input_files)
    input_files_.push_back(File(i));
}

std::vector<File> Init::getTemplates()
{
  vector<File> file_list;
  try
  {
    if (fs::exists(templates_path_) && fs::is_directory(templates_path_))
      for (const auto &entry : fs::directory_iterator(templates_path_))
        if (entry.is_regular_file())
          file_list.push_back(File(entry.path().string()));
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return file_list;
}

int Init::execute()
{
  vector<File> templates = getTemplates();
  for (const auto &f : files_)
  {
    if (f.exists() && !force_)
      if (!ask("The file " + f.getPath_() +
               " already exists. Do you want to replace it ?"))
        continue;
    for (const auto &t : templates)
    {
      if (f.getLanguage_() == C && !input_files_.empty())
      {
        if (!writeCDecls(f))
          throw ZCError(ZC_WRITING_ERROR,
                        "The file couldn't be written: " + f.getPath_());
      }
      else if (f.getExt() == t.getExt())
        f.copy(t);
      else if (f.getLanguage_() != OTHER &&
               f.getLanguage_() == t.getLanguage_())
        f.copy(t);
      else
        throw ZCError(ZC_UNSUPPORTED_LANGUAGE,
                      "No template is available for the file: " + f.getPath_());
    }
  }
  if (settings_.edit_on_init_ || edit_)
  {
    stringstream cmd;
    cmd << settings_.editor_;
    for (const auto &f : files_)
      cmd << " " << f;
    return system(cmd.str().c_str());
  }
  return 0;
}

bool Init::writeCDecls(const File &f) const
{
  Declarations all_decls;
  auto merge_decls = [&](const vector<string> &src, vector<string> &dest)
  {
    unordered_set<string> existing(dest.begin(), dest.end());

    for (const auto &item : src)
    {
      if (existing.find(item) == existing.end())
      {
        dest.push_back(item);
        // src can have the item twice too
        existing.insert(item);
      }
    }
  };

  for (const auto &f : input_files_)
  {
    unique_ptr<Declarations> d = f.parse();
    merge_decls((*d)["includes"], all_decls["includes"]);
    merge_decls((*d)["macros"], all_decls["macros"]);
    merge_decls((*d)["globals"], all_decls["globals"]);
    merge_decls((*d)["typedefs"], all_decls["typedefs"]);
    merge_decls((*d)["enums"], all_decls["enums"]);
    merge_decls((*d)["functions"], all_decls["functions"]);
    merge_decls((*d)["structs"], all_decls["structs"]);
    merge_decls((*d)["unions"], all_decls["unions"]);
  }
  return f.writeDeclarations(all_decls);
}
