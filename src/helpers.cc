#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <helpers.hh>
#include <sstream>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

fs::path getZCRootDir()
{
#if defined(_WIN32) || defined(_WIN64)
  const char *home = getenv("USERPROFILE");
#else
  const char *home = getenv("HOME");
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

string join(const vector<string> &v, const string &separator)
{
  stringstream s;
  for (int i = 0; i < v.size(); i++)
  {
    if (!(i == 0))
      s << separator;
    s << v[i];
  }
  return s.str();
}

vector<string> split(const string &s, char delimiter)
{
  vector<string> tokens;
  string token;
  istringstream tokenStream(s);
  while (getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::string upper(const std::string &s)
{
  stringstream output;
  for (const auto &c : s)
  {
    output << toupper(c);
  }
  return output.str();
}
