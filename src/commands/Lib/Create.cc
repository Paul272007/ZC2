#include <filesystem>
#include <vector>

#include <commands/Lib/Create.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Create::Create(const string &package_name, const vector<string> &files,
               bool force)
    : package_name_(package_name), force_(force),
      registry_(Registry::getInstance())
{
  for (const auto &f : files)
    files_.push_back(File(f));
}

int Create::execute()
{
  bool is_cpp = false;
  // 1. Sort files (.c, .h, .o)
  vector<File> sources, headers, objects;
  for (const auto &f : files_)
  {
    switch (f.getLanguage_())
    {
    case C:
    case INSTANCE:
    case ASSEMBLER:
      sources.push_back(f);
      break;
    case CPP:
      is_cpp = true;
      sources.push_back(f);
      break;
    case H:
    case HPP:
      headers.push_back(f);
    case OBJECT:
      objects.push_back(f);
      break;
    default:
      break;
    }
  }

  if (sources.empty() && headers.empty())
    throw ZCError(ZC_BAD_COMMAND,
                  "At least one header or source file expected.");

  // 2. Create a library by asking to the Registry to do it
  Package pkg;
  pkg.name_ = package_name_;
  pkg.version_ = "0.0.1";
  pkg.author_ = "localuser";
  pkg.flags_ = "-l" + package_name_;

  vector<fs::path> headers_paths, objects_paths, sources_paths;
  for (const auto &h : headers)
    headers_paths.push_back(fs::path(h.getPath_()));
  for (const auto &o : objects)
    objects_paths.push_back(fs::path(o.getPath_()));
  for (const auto &s : sources)
    sources_paths.push_back(fs::path(s.getPath_()));

  registry_.savePackage(pkg, force_, headers_paths, objects_paths,
                        sources_paths, is_cpp);

  return 0;
}

bool Create::isShared() const
{
  return 0;
  // return output_name_;
}
