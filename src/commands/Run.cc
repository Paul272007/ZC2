#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include <commands/Run.hh>
#include <helpers.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

#define DEBUG_MODE

using namespace std;
namespace fs = std::filesystem;

Run::Run(const std::vector<std::string> &files,
         const std::vector<std::string> &args, bool keep, bool plus,
         bool preprocess, bool compile, bool assemble)
    : plus_(plus), settings_(Settings::getInstance()),
      registry_(Registry::getInstance())
{
  // 1. Check if CPP was given and that files have correct extensions
  string badFile;
  if (isCppAndCheckExtensions(badFile))
    plus_ = true;

  if (!badFile.empty())
    throw ZCError(ZC_UNSUPPORTED_LANGUAGE,
                  "File has an uncorrect extension: " + badFile);

  // 2. Fill files_
  for (const auto &f : files)
    files_.push_back(File(f));
}

int Run::execute()
{
  // 1. Check that all files exist (file extensions were already checked before)
  string badFile = "";
  if (!filesExist(badFile))
    throw ZCError(ZC_NOT_FOUND, "File not found: " + badFile);

  string output_name = "", build_cmd = "";

  // 2. Build the compiling command following the given options
  switch (mode_)
  {
  case PREPROCESS:
    output_name =
        fs::path(files_[0].getPath_()).replace_extension(".i").string();
    break;
  case COMPILE:
    output_name =
        fs::path(files_[0].getPath_()).replace_extension(".s").string();
    break;
  case ASSEMBLE:
    output_name =
        fs::path(files_[0].getPath_()).replace_extension(".o").string();
    break;
  case FULL:
    // default:
    output_name = fs::path(files_[0].getPath_()).replace_extension("").string();
    break;
  }

  build_cmd = buildCommand(output_name);

#ifdef DEBUG_MODE
  debug("Build command: " + build_cmd);
#endif

  cout << flush;
  // TODO : get command output in a variable instead of stdout for better
  // display

  // 3. Compile program
  int compile_res = system(build_cmd.c_str());

  if (compile_res != 0)
    throw ZCError(ZC_COMPILATION_ERROR, "Compilation failed");

  success("Compilation successful.");
  if (!(mode_ == FULL))
  {
    success("File created: " + output_name);
    return 0;
  }

  // if (config_.clear_before_run_)
  //   system("clear");

  // 4. Execute program
  info("Executing program");
  string exec_cmd = fs::absolute(output_name).string();

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  int run_res = system(exec_cmd.c_str());

  if (!settings_.auto_keep_ && !keep_ && fs::exists(output_name))
  {
    fs::remove(output_name);

#ifdef DEBUG_MODE
    debug("Temporary file removed: " + output_name);
#endif
  }

  return (run_res == 0) ? 0 : 6;
}

Mode Run::getMode(bool preprocess, bool compile, bool assemble) const
{
  int flags_found = 0;
  Mode mode(FULL);
  if (preprocess)
  {
    flags_found++;
    mode = PREPROCESS;
  }
  if (compile)
  {
    flags_found++;
    mode = COMPILE;
  }
  if (assemble)
  {
    flags_found++;
    mode = ASSEMBLE;
  }
  if (flags_found > 1)
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Incompatible options");
  return mode;
}

bool Run::isCppAndCheckExtensions(string &badFile) const
{
  bool found = false;
  for (const auto &f : files_)
    switch (f.getLanguage_())
    {
    case CPP:
      found = true;
      break;
    case C:
    case OBJECT:
    case ASSEMBLER:
    case INSTANCE:
      break;
    default:
      badFile = f.getPath_();
      return false;
      break;
    }
  return found;
}

string Run::buildCommand(const string &output_name) const
{
  stringstream cmd;
  // Compiler and standard
  if (plus_)
    cmd << settings_.cpp_compiler_ << " '-std=" << settings_.cpp_std_ << "' ";
  else
    cmd << settings_.c_compiler_ << " '-std=" << settings_.c_std_ << "' ";

  // User flags
  for (const auto &f : settings_.flags_)
    cmd << escape_shell_arg(f) << " ";

  cmd << "-I" << escape_shell_arg(registry_.getIncludeDir()) << " ";
  cmd << "-L" << escape_shell_arg(registry_.getLibDir()) << " ";

  // On build mode : use map header -> lib provided by the registry

  if (mode_ == FULL)
    cmd << "-Wl,-rpath," << escape_shell_arg(registry_.getLibDir()) << " ";

  // Source files
  for (const auto &file : files_)
    cmd << file << " ";

  // Output
  cmd << "-o " << escape_shell_arg(output_name) << " ";

  // Mode and libraries for normal mode
  switch (mode_)
  {
  case PREPROCESS:
    cmd << "-E ";
    break;
  case COMPILE:
    cmd << "-S ";
    break;
  case ASSEMBLE:
    cmd << "-c ";
    break;
  default:
    const vector<string> includes = getInclusions();
    for (const auto &include : includes)
      cmd << escape_shell_arg(include) << " ";
    break;
  }

  // Color flags
  cmd << "-fdiagnostics-color=always";
  return cmd.str();
}

bool Run::filesExist(string &badFile) const
{
  for (const auto &f : files_)
  {
    if (!f.exists())
    {
      badFile = f.getPath_();
      return false;
    }
  }
  return true;
}

vector<string> Run::getInclusions() const
{
  vector<string> flags;

  for (const auto &f : files_)
  {
    vector<string> includes = f.getInclusions(registry_);
    for (const auto &include : includes)
      if (find(flags.begin(), flags.end(), include) == flags.end())
        flags.push_back(include);
  }
  return flags;
}
