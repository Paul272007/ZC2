#include <filesystem>
#include <vector>

#include <objects/Registry.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;
namespace fs = std::filesystem;

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

void Registry::load() {}

void Registry::savePackage(Package &package, bool force,
                           vector<fs::path> &headers, vector<fs::path> &objects,
                           vector<fs::path> &sources)
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
  compileObjects(sources, created_objects);
  for (const auto &obj : created_objects)
    objects.push_back(obj);

  // 5. Compile all object files into libraries
  if (!objects.empty())
  {
    createStaticLib(static_path.string(), objects);
    createSharedLib(shared_path.string(), objects);

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
  // TODO : write packages_ into the file
}
