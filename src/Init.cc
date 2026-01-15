#include <cstdlib>
#include <memory>
#include <string>

#include <File.hh>
#include <Init.hh>
#include <ZCError.hh>
#include <helpers.hh>
#include <unordered_set>
#include <vector>

using namespace std;

Init::Init(string &output_file, bool force, vector<string> &input_files,
           bool edit)
    : force_(force), output_file_(File(output_file)), edit_(edit)
{
  if (output_file_.getLanguage_() == UNSUPPORTED)
    throw ZCError(ZC_UNSUPPORTED_TYPE, "The file has an unsupported extension");

  // Get potential input files
  if (input_files.empty())
    has_input_ = false;
  else
  {
    has_input_ = true;
    for (const auto &file : input_files)
      input_files_.push_back(File(file));
  }
}

Init::~Init() {}

int Init::execute()
{
  if (!force_ && output_file_.exists())
    if (!ask("The file " + output_file_.getPath_() +
             " already exists. Do you want to replace it ?"))
      return 8;
  write();

  if (config_.open_on_init_ || edit_)
  {
    string cmd =
        config_.editor_ + " " + escape_shell_arg(output_file_.getPath_());
    return system(cmd.c_str());
  }
  return 0;
}

bool Init::write() const
{
  switch (output_file_.getLanguage_())
  {
  case PYTHON:
    return output_file_.initPython();
    break;
  case C:
    return output_file_.initC();
    break;
  case CPP:
    return output_file_.initCPP();
    break;
  case C_HEADER:
    return (has_input_) ? writeCDecls() : output_file_.initH();
    break;
  case CPP_HEADER:
    return output_file_.initHPP();
    break;
  default: // UNSUPPORTED : should not happen but we never know
    return 7;
    break;
  }
}

int Init::writeCDecls() const
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
  return (output_file_.writeDeclarations(all_decls) == true) ? 0 : 9;
}
