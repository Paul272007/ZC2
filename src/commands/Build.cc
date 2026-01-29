#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <commands/Build.hh>
#include <objects/File.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;
namespace fs = std::filesystem;

Build::Build(bool force, bool release_mode)
    : force_(force), release_mode_(release_mode),
      registry_(Registry::getInstance()), settings_(Settings::getInstance())
{
}

int Build::execute()
{
  auto sources = scanSources();
  if (sources.empty())
    throw ZCError(ZC_NO_SOURCE_FILES, "No source file was detected");

  auto libs = detectLibraries(sources);

  if (force_ || fs::exists("Cmakelists.txt"))
  {
    info("Generating CmakeLists.txt...");
    generateCMakeLists(sources, libs);
  }

  string build_type = release_mode_ ? "Release" : "Debug";
  string config_cmd = "cmake -B build -DCMAKE_BUILD_TYPE=" + build_type;
  string build_cmd = "cmake --build build";

  info("Configuring project...");
  if (system(config_cmd.c_str()) != 0)
    throw ZCError(ZC_CMAKE_ERROR, "CMake configuration failed");

  info("Building project...");
  if (system(build_cmd.c_str()) != 0)
    throw ZCError(ZC_COMPILATION_ERROR, "Build failed");

  success("Project was built successfully in build/");
  return 0;
}

vector<string> Build::detectLibraries(const std::vector<File> &sources) const
{
  vector<string> libs;
  for (const auto &file : sources)
  {
    vector<string> file_libs = file.getInclusions(registry_);

    for (const auto &lib : file_libs)
      if (find(libs.begin(), libs.end(), lib) == libs.end())
        libs.push_back(lib);
  }
  return libs;
}

bool Build::generateCMakeLists(const vector<File> &sources,
                               const vector<string> &libs)
{
  ofstream cmake("CMakeLists.txt");
  if (!cmake.is_open())
  {
    throw ZCError(ZC_WRITING_ERROR, "Could not write CMakeLists.txt");
    return false;
  }
  string project_name = fs::current_path().filename().string();

  cmake << "cmake_minimum_required(VERSION 3.12)\n";
  cmake << "project(" << project_name << " C CXX)\n\n";

  // Standards
  cmake << "set(CMAKE_CXX_STANDARD " << (settings_.cpp_std_.substr(3)) << ")\n";
  cmake << "set(CMAKE_C_STANDARD " << (settings_.c_std_.substr(1)) << ")\n\n";

  // --- ZC Integration ---
  cmake << "# ZC Paths\n";
  cmake << "include_directories(" << registry_.getIncludeDir().string()
        << ")\n";
  cmake << "link_directories(" << registry_.getLibDir().string() << ")\n\n";

  // Source code
  cmake << "add_executable(" << project_name << '\n';
  for (const auto &src : sources)
    cmake << "    " << src << "\n";
  cmake << ")\n";

  // Linking
  if (!libs.empty())
  {
    cmake << "target_link_libraries(" << project_name << " PRIVATE\n";
    for (const auto &lib : libs)
    {
      cmake << "    " << lib << "\n";
    }
    cmake << ")\n";
  }

  // Add de pthread/dl if Linux
  // cmake << "if(UNIX)\n";
  // cmake << "    target_link_libraries(" << project_name << " PRIVATE pthread
  // dl)\n"; cmake << "endif()\n";

  cmake << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";

  cmake.close();
  return true;
}
