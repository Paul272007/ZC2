#include <filesystem>
#include <sstream>

#include <Config.hh>
#include <File.hh>
#include <Run.hh>
#include <ZCError.hh>
#include <helpers.hh>

#define DEBUG_MODE

using namespace std;
namespace fs = std::filesystem;

Run::Run(const std::vector<std::string> &files,
         const std::vector<std::string> &args, bool keep, bool plus,
         bool preprocess, bool compile, bool assemble)
    : args_(args), keep_(keep), plus_(plus)
{
  for (const auto &f : files)
    files_.push_back(File(f));
  if (isCpp())
    plus_ = true;
  mode_ = getMode(preprocess, compile, assemble);
}

Run::~Run() {}

int Run::execute()
{
  string badFile = "";
  // Check that all files exist
  if (!filesExist(badFile))
    throw ZCError(ZC_FILE_NOT_FOUND, "File not found: " + badFile);
  if (!filesHaveCorrectExt(badFile))
    throw ZCError(ZC_UNSUPPORTED_TYPE,
                  "File has an uncorrect extension: " + badFile);
  string output_name = "", build_cmd = "";
  int height, width;

  // Build command following the given options
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
  default: // case FULL:
    output_name = findMainFile();
    break;
  }
  if (output_name.empty())
    throw ZCError(ZC_NO_MAIN_FUNCTION,
                  "No main function was found in the given files");
  buildCommand(build_cmd, output_name);
#ifdef DEBUG_MODE
  debug("Build command: " + build_cmd);
#endif
  // TODO : make better displaying with the C display.h library
  cout << flush;
  // TODO : get command output in a variable instead of stdout
  int compile_res = system(build_cmd.c_str());

  if (compile_res != 0)
    throw ZCError(ZC_COMPILATION, "Compilation failed");

  success("Compilation successful.");
  if (!(mode_ == FULL))
  {
    success("File created: " + output_name);
    return 0;
  }
  // if (config_.clear_before_run_)
  //   system("clear");
  info("Executing program");
  string exec_cmd = fs::absolute(output_name).string();

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  int run_res = system(exec_cmd.c_str());

  if (!config_.auto_keep_ && !keep_ && fs::exists(output_name))
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
    throw ZCError(ZC_BAD_COMMAND, "Incompatible options");
  return mode;
}

string Run::findMainFile() const
{
  for (const auto &file : files_)
  {
    if (file.getPath_().find(".c") == string::npos &&
        file.getPath_().find(".cpp") == string::npos &&
        file.getPath_().find(".cc") == string::npos &&
        file.getPath_().find(".cxx") == string::npos)
    {
      continue;
    }

    if (file.containsMain())
    {
      size_t lastIndex = file.getPath_().find_last_of(".");
      return file.getPath_().substr(0, lastIndex);
    }
  }
  return "";
}

bool Run::isCpp() const
{
  for (const auto &f : files_)
  {
    string ext = f.getExt();
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".hpp")
    {
      return true;
    }
  }
  return false;
}

void Run::buildCommand(string &buildCmd, const string &output_name) const
{
  stringstream cmd;
  // Compiler and standard
  if (plus_)
    cmd << config_.cpp_compiler_ << " '-std=" << config_.cpp_std_ << "' ";
  else
    cmd << config_.c_compiler_ << " '-std=" << config_.c_std_ << "' ";

  // User flags
  for (const auto &f : config_.flags_)
    cmd << escape_shell_arg(f) << " ";

  cmd << "-I" << escape_shell_arg(config_.include_dir_) << " ";
  cmd << "-L" << escape_shell_arg(config_.lib_dir_) << " ";

  if (mode_ == FULL)
    cmd << "-Wl,-rpath," << escape_shell_arg(config_.lib_dir_) << " ";

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
  buildCmd = cmd.str();
}

bool Run::filesExist(string &fileNotFound) const
{
  for (const auto &f : files_)
  {
    if (!f.exists())
    {
      fileNotFound = f.getPath_();
      return false;
    }
  }
  return true;
}

bool Run::filesHaveCorrectExt(std::string &badFile) const
{
  for (const auto &f : files_)
  {
    string ext = f.getExt();
    if (ext != ".c" && ext != ".o" && ext != ".cpp" && ext != ".cxx" &&
        ext != ".cc")
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
    vector<string> includes = f.getInclusions(config_.libraries_.at("c"));
    for (const auto &include : includes)
      if (find(flags.begin(), flags.end(), include) == flags.end())
        flags.push_back(include);
  }
  return flags;
}
