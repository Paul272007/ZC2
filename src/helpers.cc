#include <cstdlib>
#include <filesystem>
#include <helpers.hh>

using namespace std;
namespace fs = std::filesystem;

fs::path get_zc_path()
{
  const char *home;

#if defined(_WIN32) || defined(_WIN64)
  home = getenv("USERPROFILE");
#else
  home = getenv("HOME");
#endif

  if (!home)
    return fs::current_path() / ROOT_DIR;

  return fs::path(home) / ROOT_DIR;
}

string escape_shell_arg(const string &arg)
{
  string escaped = "'";
  for (char c : arg)
  {
    if (c == '\'')
    {
      escaped += "'\\''";
    }
    else
    {
      escaped += c;
    }
  }
  escaped += "'";
  return escaped;
}
