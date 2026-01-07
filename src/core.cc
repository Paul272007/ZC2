#include <core.hpp>
#include <map>
#include <regex>
#include <string>
#include <vector>

using namespace std;

Config get_conf()
{
  json json_conf;
  ifstream input(PATH);
  if (input.is_open())
  {
    try
    {
      input >> json_conf;
    }
    catch (json::parse_error &e)
    {
      string msg = "JSON parse error: ";
      msg += e.what();
      throw ZCError(1, msg);
    }
  }
  Config conf = {json_conf["c_compiler"],
                 json_conf["cpp_compiler"],
                 json_conf["flags"],
                 json_conf["libraries"],
                 json_conf["clear_before_run"],
                 json_conf["auto_keep"],
                 json_conf["std"],
                 json_conf["include_dir"],
                 json_conf["lib_dir"]};
  return conf;
}

void write_conf(json conf)
{
  ofstream output(PATH);
  if (output.is_open())
  {
    try
    {
      output << setw(4) << conf << endl;
    }
    catch (exception &e)
    {
      string msg = "configuration writing: ";
      msg += e.what();
      throw ZCError(2, msg);
    }
  }
}

bool ask()
{
  char input = 'Y';
  cout << "[Y/n] ";
  cin >> input;
  while (input != 'Y' && input != 'n')
  {
    cout << "Error: unexpected token" << endl << "[Y/n] ";
    cin >> input;
  }
  return input == 'Y';
}

FileParser::FileParser(const string &path) : path_(path) {}

Declarations FileParser::parse()
{
  Declarations decl;

  string raw_content = readFile();

  if (raw_content.empty())
    return decl;

  string content = removeComments(raw_content);

  regex re_include(R"((?:^|\n)\s*#include\s+[^\r\n]*)");
  decl.includes = findAll(content, re_include);

  regex re_macro(R"((?:^|\n)\s*#define\s+[A-Z0-9_]+\s+[^\(\r\n]+)");
  decl.macros = findAll(content, re_macro);

  decl.structs = extractBlock(content, "struct");

  regex re_func(
      R"((?:^|\n)(?:[a-zA-Z0-9_]+\s+)+(?:\*+\s*)?[a-zA-Z0-9_]+\s*\([^)]*\)\s*(?=\{))");
  auto raw_funcs = findAll(content, re_func);

  for (const auto &f : raw_funcs)
  {
    // Nettoyage sommaire des retours ligne initiaux si présents à cause du
    // (?:^|\n)
    string clean_f = regex_replace(f, regex(R"(^\n)"), "");
    if (clean_f.find("main") == string::npos && clean_f.find("static") != 0)
    {
      decl.functions.push_back(clean_f);
    }
  }

  return decl;
}

string FileParser::readFile() const
{
  ifstream file(path_);

  if (!file.is_open())
    return "";
  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

vector<string> FileParser::findAll(const string &text, const regex &re)
{
  vector<string> matches;
  auto begin = sregex_iterator(text.begin(), text.end(), re);
  auto end = sregex_iterator();
  for (sregex_iterator i = begin; i != end; i++)
  {
    matches.push_back(i->str());
  }

  return matches;
}

string FileParser::removeComments(const string &text)
{
  regex re(R"((//[^\n]*)|(/\*[\s\S]*?\*/)|("(?:\\.|[^"\\])*"))");

  stringstream result;
  auto begin = sregex_iterator(text.begin(), text.end(), re);
  auto end = sregex_iterator();

  size_t last_pos = 0;

  for (sregex_iterator i = begin; i != end; i++)
  {
    std::smatch match = *i;

    result << text.substr(last_pos, match.position() - last_pos);

    if (match[1].matched || match[2].matched)
    {
      result << " ";
    }
    else if (match[3].matched)
    {
      result << match[3].str();
    }

    last_pos = match.position() + match.length();
  }

  result << text.substr(last_pos);
  return result.str();
}

vector<string> FileParser::extractBlock(const string &content,
                                        const string &keyword)
{
  string pattern_str =
      R"((?:\btypedef\s+)?\b)" + keyword + R"(\b[^{;()]*\{[\s\S]*?\}[^;]*;)";
  regex re(pattern_str);

  return findAll(content, re);
}

bool FileParser::containsMain()
{
  string raw_content = readFile();
  if (raw_content.empty())
    return false;

  string content = removeComments(raw_content);

  regex re_main(R"(\bint\s+main\s*\()");

  // std::regex_search renvoie true dès qu'une correspondance est trouvée
  return regex_search(content, re_main);
}

string findMainFile(const vector<string> &files)
{
  for (const auto &path : files)
  {
    if (path.find(".c") == string::npos && path.find(".cpp") == string::npos &&
        path.find(".cc") == string::npos && path.find(".cxx") == string::npos)
    {
      continue;
    }

    FileParser parser(path);
    if (parser.containsMain())
    {
      size_t lastIndex = path.find_last_of(".");
      return path.substr(0, lastIndex);
    }
  }
  return "";
}

vector<string>
FileParser::getInclusions(const map<string, string> &libraries) const
{
  string content = this->readFile();
  vector<string> flags;
  for (const auto [lib_name, lib_flag] : libraries)
  {
    string pattern = R"(#include\s*[<"])" + lib_name + R"(\.h[>"])";
    regex re(pattern);

    if (regex_search(content, re))
    {
      // if (find(flags.begin(), flags.end(), lib_flag) == flags.end())
      // {
      flags.push_back(lib_flag);
      // }
    }
  }
  return flags;
}
