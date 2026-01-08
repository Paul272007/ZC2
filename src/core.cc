#include <chrono>
#include <clang-c/Index.h>
#include <colors.h>
#include <core.hpp>
#include <format>
#include <map>
#include <regex>
#include <string>
#include <vector>

using namespace std;

struct VisitorContext
{
  Declarations *decls;
  const string *content;
  vector<pair<unsigned, unsigned>> typedef_ranges;
};

// ===================================================== Helpers

/**
 * @brief Helper to trim strings
 */
static void rtrim(string &s)
{
  while (!s.empty() && isspace(s.back()))
  {
    s.pop_back();
  }
}

/**
 * @brief Helper to get the (start, end) offsets of a cursor
 */
static pair<unsigned, unsigned> get_cursor_offsets(CXCursor cursor)
{
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned start_offset, end_offset;
  clang_getInstantiationLocation(start, nullptr, nullptr, nullptr,
                                 &start_offset);
  clang_getInstantiationLocation(end, nullptr, nullptr, nullptr, &end_offset);

  return {start_offset, end_offset};
}

// Helper pour extraire le texte (réutilise les offsets)
static string get_cursor_text(CXCursor cursor, const string &content)
{
  auto [start, end] = get_cursor_offsets(cursor);

  if (end <= start || end > content.length())
  {
    return "";
  }
  return content.substr(start, end - start);
}

// Vérifie si un curseur est inclus dans un typedef déjà vu
static bool is_inside_typedef(CXCursor cursor,
                              const vector<pair<unsigned, unsigned>> &ranges)
{
  auto [start, end] = get_cursor_offsets(cursor);
  for (const auto &range : ranges)
  {
    // Si l'élément commence APRES le début du typedef et finit AVANT la fin du
    // typedef
    if (start >= range.first && end <= range.second)
    {
      if (start == range.first && end == range.second)
        continue;
      return true;
    }
  }
  return false;
}

// --- Visiteurs ---

// PASSE 1 : Repérage des typedefs
static CXChildVisitResult visitor_find_typedefs(CXCursor cursor,
                                                CXCursor parent,
                                                CXClientData client_data)
{
  auto *ctx = static_cast<VisitorContext *>(client_data);
  CXCursorKind kind = clang_getCursorKind(cursor);

  // On ignore ce qui n'est pas dans le fichier principal
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  if (!clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  if (kind == CXCursor_TypedefDecl)
  {
    ctx->typedef_ranges.push_back(get_cursor_offsets(cursor));
  }

  return CXChildVisit_Continue; // On continue pour tout trouver
}

// PASSE 2 : Extraction
static CXChildVisitResult visitor_extract(CXCursor cursor, CXCursor parent,
                                          CXClientData client_data)
{
  auto *ctx = static_cast<VisitorContext *>(client_data);
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (clang_getCursorLinkage(cursor) == CXLinkage_Internal)
    return CXChildVisit_Continue;

  CXSourceLocation loc = clang_getCursorLocation(cursor);
  if (!clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  // Si c'est une structure/enum/union, on vérifie si elle est "mangée" par un
  // typedef
  if (kind == CXCursor_EnumDecl || kind == CXCursor_StructDecl ||
      kind == CXCursor_UnionDecl)
  {
    if (is_inside_typedef(cursor, ctx->typedef_ranges))
    {
      return CXChildVisit_Continue; // On l'ignore, le typedef la contient déjà
    }
  }

  // Extraction du texte
  string text = get_cursor_text(cursor, *ctx->content);
  if (text.empty())
    return CXChildVisit_Continue;

  // Dispatch selon le type
  if (kind == CXCursor_InclusionDirective)
  {
    ctx->decls->includes.push_back(text + "\n");
  }
  else if (kind == CXCursor_MacroDefinition)
  {
    if (!clang_Cursor_isMacroBuiltin(cursor))
    {
      ctx->decls->macros.push_back(text);
    }
  }
  else if (kind == CXCursor_TypedefDecl)
  {
    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();
    ctx->decls->typedefs.push_back(text);
  }
  else if (kind == CXCursor_EnumDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      ctx->decls->enums.push_back(text);
    }
  }
  else if (kind == CXCursor_StructDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      ctx->decls->structs.push_back(text);
    }
  }
  else if (kind == CXCursor_UnionDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      ctx->decls->unions.push_back(text);
    }
  }
  else if (kind == CXCursor_VarDecl)
  {
    // Nettoyage variables globales
    size_t equal_pos = text.find('=');
    if (equal_pos != string::npos)
      text = text.substr(0, equal_pos);

    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();

    if (text.find("extern") == string::npos)
      text = "extern " + text;
    ctx->decls->globals.push_back(text);
  }
  else if (kind == CXCursor_FunctionDecl)
  {
    CXString name_str = clang_getCursorSpelling(cursor);
    string name = clang_getCString(name_str);
    clang_disposeString(name_str);

    if (name != "main")
    {
      size_t brace_pos = text.find('{');
      if (brace_pos != string::npos)
        text = text.substr(0, brace_pos);

      rtrim(text);
      if (!text.empty() && text.back() == ';')
        text.pop_back();
      ctx->decls->functions.push_back(text);
    }
  }

  return CXChildVisit_Continue;
}

// ================================== Functions that will get exported

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

void success(const string &msg)
{
  cout << GREEN << "[SUCCESS] " << COLOR_RESET << msg << endl;
}

void debug(const string &msg)
{
  cout << CYAN << "[DEBUG]   " << COLOR_RESET << msg << endl;
}

void warning(const string &msg)
{
  cout << YELLOW << "[WARNING] " << COLOR_RESET << msg << endl;
}

void info(const string &msg)
{
  cout << BLUE << "[INFO]    " << COLOR_RESET << msg << endl;
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

string upper(string str)
{
  for (char &c : str)
  {
    c = toupper(c);
  }
  return str;
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

// ================================== FileParser Class

FileParser::FileParser(const string &path) : path_(path) {}

string FileParser::readFile() const
{
  ifstream file(path_);

  if (!file.is_open())
    return "";
  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

Declarations FileParser::parse()
{
  Declarations decls;

  // 1. Lire le contenu du fichier pour l'extraction de texte
  string content = readFile();
  if (content.empty())
    return decls;

  // 2. Initialiser l'index libclang
  CXIndex index = clang_createIndex(0, 0);

  // 3. Arguments de compilation (très important pour les headers)
  // On ajoute le dossier include courant et on force le mode C
  const char *args[] = {"-x", "c", "-I.", "-Iinclude"};

  // 4. Parser le fichier
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path_.c_str(), args, std::size(args), nullptr, 0,
      CXTranslationUnit_DetailedPreprocessingRecord |
          CXTranslationUnit_KeepGoing);

  if (unit == nullptr)
  {
    // Fallback ou erreur silencieuse, selon votre besoin.
    // Ici on lance l'erreur comme dans votre code original.
    clang_disposeIndex(index);
    throw ZCError(9, "Unable to parse translation unit: " + path_);
  }

  // 5. Lancer le visiteur
  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  VisitorContext ctx = {&decls, &content};
  // clang_visitChildren(cursor, visitor, &ctx);
  // 1
  clang_visitChildren(cursor, visitor_find_typedefs, &ctx);
  // 2
  clang_visitChildren(cursor, visitor_extract, &ctx);

  // 6. Nettoyage
  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return decls;
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

bool FileParser::exists() const
{
  ifstream file(path_);

  return (file.is_open());
}

bool FileParser::writeContent(const string &content) const
{
  ofstream file(path_);

  if (!file.is_open())
    return false;

  file << content;
  if (file.good())
  {
    success("File written: " + path_);
    return true;
  }
  throw ZCError(7, "Error writing to file: " + path_);
  return false;
}

bool FileParser::appendContent(const string &content) const
{
  ofstream file(path_, ios::app);

  if (!file.is_open())
    return false;

  file << content;
  return file.good();
}

bool FileParser::writeDeclarations(const Declarations &decls,
                                   const string &constant) const
{
  stringstream content;

  // Custom header
  content << "/*\n\tThis file was automatically generated by ZC\n";
  auto now = chrono::system_clock::now();
  string s = format("{:%F %T}", now);
  content << "\tDate of creation: " << s << '\n';
  content << "\tEditing this file manually could break it.\n*/\n\n";

  // Header guards
  content << "#ifndef " << constant << '\n';
  content << "#define " << constant << "\n\n";

  if (!decls.includes.empty())
  {
    content << "/* Includes */\n";
    for (const auto &inc : decls.includes)
    {
      content << inc;
    }
    content << '\n';
  }

  if (!decls.macros.empty())
  {
    content << "/* Macros */\n";
    for (const auto &macro : decls.macros)
    {
      content << "#define " << macro << '\n';
    }
    content << '\n';
  }

  if (!decls.enums.empty())
  {
    content << "/* Enums */\n";
    for (const auto &en : decls.enums)
    {
      content << en << ";\n";
    }
    content << '\n';
  }

  if (!decls.unions.empty())
  {
    content << "/* Unions */\n";
    for (const auto &un : decls.unions)
    {
      content << un << ";\n";
    }
    content << '\n';
  }

  if (!decls.structs.empty())
  {
    content << "/* Structures */\n";
    for (const auto &struc : decls.structs)
    {
      content << struc << ";\n";
    }
    content << '\n';
  }

  if (!decls.typedefs.empty())
  {
    content << "/* Typedefs */\n";
    for (const auto &td : decls.typedefs)
    {
      content << td << ";\n";
    }
    content << '\n';
  }

  if (!decls.globals.empty())
  {
    content << "/* Global variables */\n";
    for (const auto &glob : decls.globals)
    {
      content << glob << ";\n";
    }
    content << '\n';
  }

  if (!decls.functions.empty())
  {
    content << "/* Functions */\n";
    for (const auto &func : decls.functions)
    {
      content << func << ";\n";
    }
    content << '\n';
  }

  content << "\n#endif // !" << constant << "\n";

  return writeContent(content.str());
}

FileParser::~FileParser() {}
