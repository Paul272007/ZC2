#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#include <LibCreate.hh>
#include <ZCError.hh>
#include <helpers.hh>

using namespace std;
namespace fs = std::filesystem;

LibCreate::LibCreate(const string &output_name, const vector<string> &files,
                     bool force)
    : output_name_(output_name), files_(files), force_(force)
{
  // Do not load libraries for the moment
}

LibCreate::~LibCreate() {}

bool LibCreate::isShared() const
{
  // Guess type based on the output extension
  return output_name_.find(".so") != string::npos ||
         output_name_.find(".dylib") != string::npos ||
         output_name_.find(".dll") != string::npos;
}

void LibCreate::compileObjects(const vector<string> &sources,
                               vector<string> &objects, bool shared) const
{
  if (sources.empty())
    return;

  string compiler = config_.cpp_compiler_; // Use cpp compiler for the moment

  for (const auto &src : sources)
  {
    string obj = fs::path(src).stem().string() + ".o";
    stringstream cmd;
    cmd << compiler << "-c " << escape_shell_arg(src) << " -o " << obj;

    if (shared)
      cmd << " -fPIC";

    info("Compiling " + src + "...");
    if (system(cmd.str().c_str()) != 0)
      throw ZCError(ZC_COMPILATION, "Compilation failed for: " + src);

    objects.push_back(obj);
  }
}
int LibCreate::execute()
{
  if (files_.empty())
    throw ZCError(ZC_BAD_COMMAND, "No input files provided.");

  // 1. Séparation des sources (.c/.cpp) et des objets déjà compilés (.o)
  vector<string> sources_to_compile;
  vector<string> objects;

  for (const auto &f : files_)
  {
    string ext = fs::path(f).extension().string();
    if (ext == ".c" || ext == ".cc" || ext == ".cpp" || ext == ".cxx")
      sources_to_compile.push_back(f);
    else if (ext == ".o")
      objects.push_back(f);
    else
      warning("Ignoring unknown file type" + f);
  }

  // 2. Compiler ce qui doit l'être
  bool shared = isShared();
  compileObjects(sources_to_compile, objects, shared);

  if (objects.empty())
    throw ZCError(ZC_BAD_COMMAND, "Nothing to link/archive.");

  // 3. Création de la librairie
  stringstream cmd;

  if (shared)
  {
    // --- MODE PARTAGÉ (.so) ---
    // On utilise le compilateur pour lier
    cmd << config_.cpp_compiler_ << " -shared -o "
        << escape_shell_arg(output_name_);
    for (const auto &obj : objects)
      cmd << " " << obj;

    cout << "[INFO] Linking shared library: " << output_name_ << endl;
  }
  else
  {
    // --- MODE STATIQUE (.a) ---
    // On utilise 'ar'
    cmd << "ar rcs " << escape_shell_arg(output_name_);
    for (const auto &obj : objects)
      cmd << " " << obj;

    cout << "[INFO] Archiving static library: " << output_name_ << endl;
  }

  int res = system(cmd.str().c_str());

  // Nettoyage des .o temporaires générés par nous
  if (!config_.auto_keep_)
  {
    for (const auto &src : sources_to_compile)
    {
      string obj = fs::path(src).stem().string() + ".o";
      if (fs::exists(obj))
        fs::remove(obj);
    }
  }

  if (res != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Library creation failed.");

  cout << "\033[1;32m[SUCCESS] Library created: " << output_name_ << "\033[0m"
       << endl;
  return 0;
}
