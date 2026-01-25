#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

Registry &Registry::getInstance()
{
  static Registry instance;
  static bool initialized = false;
  if (!initialized)
  {
    instance.load();
    initialized = true;
  }
  return instance;
}

void from_json(const json &j, Package &p)
{
  j.at(0).get_to(p.name_);     // Index 0: "math"
  j.at(1).get_to(p.headers_);  // Index 1: ["math.h"]
  j.at(2).get_to(p.binaries_); // Index 2: []
  j.at(3).get_to(p.flags_);    // Index 3: "-lm"
  j.at(4).get_to(p.version_);  // Index 4: "0.0.0"
  j.at(5).get_to(p.author_);   // Index 5: "std"
}

void to_json(json &j, const Package &p)
{
  j = json::array({
      p.name_,     // Index 0
      p.headers_,  // Index 1
      p.binaries_, // Index 2
      p.flags_,    // Index 3
      p.version_,  // Index 4
      p.author_    // Index 5
  });
}

void Registry::load()
{
  json json_conf;
  if (!fs::exists(registry_path_))
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND,
                  "The configuration file was not found: " +
                      registry_path_.string());
    return;
  }
  ifstream input(registry_path_);
  if (!input.is_open())
    throw ZCError(ZC_CONFIG_READING_ERROR,
                  "The configuration file couldn't be read: " +
                      registry_path_.string());
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(ZC_CONFIG_PARSING_ERROR,
                  "The configuration file couldn't be parsed: " +
                      registry_path_.string() + ": " + e.what());
  }
  vector<Package> packages = json_conf.at("libraries").get<vector<Package>>();
}

void Registry::savePackage(Package &package, bool force,
                           vector<fs::path> &headers, vector<fs::path> &objects,
                           vector<fs::path> &sources, bool is_cpp)
{
  // 1. Create package subdirectory for headers and check if it already exists
  // (which means the library already exists)
  fs::path package_dir = include_path_ / package.name_;
  if (!force && fs::exists(package_dir))
    if (!ask("The library " + (package.name_) +
             " already exists. Do you want to replace it ?"))
    {
      throw ZCError(ZC_OPERATIONS_ABORTED, "Operations aborted.");
      return;
    }
  fs::create_directories(package_dir);

  // 2. Install header files
  vector<string> installed_headers;
  for (const auto &h : headers)
  {
    fs::create_directories(package_dir);
    fs::path dest = package_dir / h;

    fs::copy(h, dest, fs::copy_options::overwrite_existing);
    package.headers_.push_back(h.string());
  }

  // 3. Build library names (static + shared)
  string lib_base = "lib" + package.name_;
  fs::path static_path = lib_path_ / (lib_base + ".a");

#if defined(_WIN32) || defined(_WIN64)
  string shared_ext = ".dll";
#elif defined(__APPLE__)
  string shared_ext = ".dylib";
#else
  string shared_ext = ".so";
#endif
  fs::path shared_path = lib_path_ / (lib_base + shared_ext);

  // 4. Compile all source code into object files
  vector<fs::path> created_objects;
  compileObjects(sources, created_objects, is_cpp);
  for (const auto &obj : created_objects)
    objects.push_back(obj);

  // 5. Compile all object files into libraries
  if (!objects.empty())
  {
    createStaticLib(static_path.string(), objects);
    createSharedLib(shared_path.string(), objects, is_cpp);

    for (const auto &obj : created_objects)
      fs::remove(obj);
  }

  if (fs::exists(static_path))
    package.binaries_.push_back(static_path);
  if (fs::exists(shared_path))
    package.binaries_.push_back(shared_path);

  // 6. Index the library in the config file
  indexPackage(package);
}

void Registry::indexPackage(const Package &package)
{
  packages_.push_back(package);
  json root;
  root["libraries"] = packages_;
  ofstream output(registry_path_);
  if (!output.is_open())
    throw ZCError(ZC_CONFIG_WRITING_ERROR,
                  "The registry couldn't be written: " +
                      registry_path_.string());
  output << root.dump(4);
  output.close();
}

Table Registry::packagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Author", "Version",
                                   "Compiling flags", "Headers", "Binaries"}};

  for (const auto &p : packages_)
    str_pkgs.push_back({p.name_, p.author_, p.version_, p.flags_});

  return Table(packages_.size() + 1, N_ATTR_PACKAGE, false, true, str_pkgs);
}

fs::path Registry::getIncludeDir() const { return include_path_; }

fs::path Registry::getLibDir() const { return lib_path_; }

std::vector<Package> Registry::getPackages() const { return packages_; }

void Registry::compileObjects(const std::vector<std::filesystem::path> &sources,
                              std::vector<std::filesystem::path> &objects,
                              bool is_cpp) const
{
  // Compile each file separately
  for (const auto &s : sources)
  {
    fs::path obj = s;
    stringstream cmd;
    cmd << (is_cpp ? "g++" : "gcc") << " -c -fPIC " << s.string() << "-o "
        << obj.replace_extension(".o").string();
    if (system(cmd.str().c_str()) != 0)
      throw ZCError(ZC_COMPILATION_ERROR, "An error occured while compiling " +
                                              s.string() + " to " +
                                              obj.string());
    objects.push_back(obj);
  }
}

bool Registry::createStaticLib(
    const std::string &libPath,
    const std::vector<std::filesystem::path> &objects) const
{
  stringstream cmd;
  cmd << "ar rcs" << libPath;
  for (const auto &o : objects)
    cmd << " " << o;
  return (system(cmd.str().c_str()) == 0);
}

bool Registry::createSharedLib(const std::string &libPath,
                               const std::vector<fs::path> &objects,
                               bool is_cpp) const
{
  stringstream cmd;
  cmd << (is_cpp ? "g++" : "gcc");
#ifdef __APPLE__
  cmd << " -dynamiclib ";
#else
  cmd << " -shared ";
#endif
  cmd << libPath;
  for (const auto &o : objects)
    cmd << o.string();
  return system(cmd.str().c_str()) == 0;
}

vector<string> Registry::unindexPackage(const std::string &pkg_name)
{
  vector<string> binaries;
  // 1. Find package
  auto it = find_if(packages_.begin(), packages_.end(),
                    [&](const Package &p) { return p.name_ == pkg_name; });

  // 2. Check if package was found
  if (it != packages_.end())
  {
    binaries = it->binaries_;

    // 3. Delete package
    packages_.erase(it);
  }
  else
  {
    throw ZCError(ZC_PACKAGE_NOT_FOUND,
                  "The package was not found: " + pkg_name);
  }

  nlohmann::json root;
  root["libraries"] = packages_;

  std::ofstream output(registry_path_);
  if (!output.is_open())
    throw ZCError(ZC_CONFIG_WRITING_ERROR,
                  "The registry couldn't be written: " +
                      registry_path_.string());

  output << root.dump(4);
  output.close();

  return binaries;
}

bool Registry::removePackage(const std::string &pkg_name)
{
  vector<string> binaries = unindexPackage(pkg_name);

  if (fs::exists(include_path_ / pkg_name))
    fs::remove_all(include_path_ / pkg_name);
  else
    return false;
  for (const auto &b : binaries)
  {
    if (fs::exists(lib_path_ / b))
      fs::remove(b);
    else
      return false;
  }
  return true;
}
