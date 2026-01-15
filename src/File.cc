#include <chrono>
#include <clang-c/Index.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include <Config.hh>
#include <File.hh>
#include <ZCError.hh>
#include <helpers.hh>

using namespace std;
namespace fs = std::filesystem;

// ----------------------------------------------- Helpers

namespace
{

struct VisitorContext
{
  Declarations *decls;
  const string *content;
  vector<pair<unsigned, unsigned>> typedef_ranges;
};

struct MainSearchContext
{
  bool found = false;
};

/**
 * @brief Helper to trim strings
 */
void rtrim(string &s)
{
  while (!s.empty() && isspace(s.back()))
  {
    s.pop_back();
  }
}

/**
 * @brief Helper to get the (start, end) offsets of a cursor
 */
pair<unsigned, unsigned> get_cursor_offsets(CXCursor cursor)
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
string get_cursor_text(CXCursor cursor, const string &content)
{
  auto [start, end] = get_cursor_offsets(cursor);

  if (end <= start || end > content.length())
  {
    return "";
  }
  return content.substr(start, end - start);
}

// Vérifie si un curseur est inclus dans un typedef déjà vu
bool is_inside_typedef(CXCursor cursor,
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
CXChildVisitResult visitor_find_typedefs(CXCursor cursor, CXCursor parent,
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
CXChildVisitResult visitor_extract(CXCursor cursor, CXCursor parent,
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
    (*ctx->decls)["includes"].push_back(text + "\n");
  }
  else if (kind == CXCursor_MacroDefinition)
  {
    if (!clang_Cursor_isMacroBuiltin(cursor))
    {
      (*ctx->decls)["macros"].push_back(text);
    }
  }
  else if (kind == CXCursor_TypedefDecl)
  {
    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();
    (*ctx->decls)["typedefs"].push_back(text);
  }
  else if (kind == CXCursor_EnumDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["enums"].push_back(text);
    }
  }
  else if (kind == CXCursor_StructDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["structs"].push_back(text);
    }
  }
  else if (kind == CXCursor_UnionDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["unions"].push_back(text);
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
    (*ctx->decls)["globals"].push_back(text);
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
      (*ctx->decls)["functions"].push_back(text);
    }
  }

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor_find_main(CXCursor cursor, CXCursor parent,
                                     CXClientData client_data)
{
  auto *context = static_cast<MainSearchContext *>(client_data);

  // On ne cherche que les déclarations de fonctions
  if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)
  {
    CXString spelling = clang_getCursorSpelling(cursor);
    string name = clang_getCString(spelling);

    if (name == "main")
    {
      context->found = true;
      clang_disposeString(spelling);
      return CXChildVisit_Break; // On arrête la visite dès qu'on a trouvé
    }
    clang_disposeString(spelling);
  }

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor_find_includes(CXCursor cursor, CXCursor parent,
                                         CXClientData client_data)
{
  auto *includes_vec = static_cast<vector<string> *>(client_data);

  // On ne cible que les directives #include
  if (clang_getCursorKind(cursor) == CXCursor_InclusionDirective)
  {
    CXString spelling = clang_getCursorSpelling(cursor);
    string name = clang_getCString(spelling);

    // On ajoute l'inclusion à notre liste
    includes_vec->push_back(name);

    clang_disposeString(spelling);
  }

  return CXChildVisit_Continue;
}

} // namespace

// ----------------------------------------------- File class

File::File(string filepath) : path_(filepath)
{
  string ext = getExt();
  if (ext == ".py")
    language_ = PYTHON;
  else if (ext == ".pyc")
    language_ = PYC;
  else if (ext == ".c")
    language_ = C;
  else if (ext == ".h")
    language_ = C_HEADER;
  else if (ext == ".cpp" || ext == ".cc" || ext == ".cxx")
    language_ = CPP;
  else if (ext == ".hpp" || ext == ".hh" || ext == ".hxx")
    language_ = CPP_HEADER;
  else if (ext == ".a")
    language_ = ARCHIVE;
  else if (ext == ".so")
    language_ = DYNAMIC_LIB;
  else
    language_ = UNSUPPORTED;
}

File::~File() {}

string File::getPath_() const { return path_; }

Language File::getLanguage_() const { return language_; }

string File::getFilename() const { return split(path_, '/').back(); }

string File::readFile() const
{
  ifstream file(path_);

  if (!file.is_open())
    return "";
  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool File::writeContent(const string &content) const
{
  ofstream file(path_);

  if (!file.is_open())
  {
    throw ZCError(ZC_FILE_NOT_FOUND, "File not found: " + path_);
    return false;
  }

  file << content;
  if (!file.good())
  {
    throw ZCError(ZC_FILE_WRITING_ERROR, "Error writing to file: " + path_);
    return false;
  }
  success("File written: " + path_);
  return true;
}

unique_ptr<Declarations> File::parse() const
{
  unique_ptr<Declarations> decls = make_unique<Declarations>();

  // 1. Read file and extract content
  string content = readFile();
  if (content.empty())
    return decls;

  // 2. Initialize libclang index
  CXIndex index = clang_createIndex(0, 0);

  // 3. Arguments de compilation (très important pour les headers)
  // On ajoute le dossier include courant et on force le mode C
  const char *args[] = {"-x", "c", "-I.", "-Iinclude"};

  // 4. Parser le fichier
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path_.c_str(), args, size(args), nullptr, 0,
      CXTranslationUnit_DetailedPreprocessingRecord |
          CXTranslationUnit_KeepGoing);

  if (unit == nullptr)
  {
    // Fallback ou erreur silencieuse, selon votre besoin.
    // Ici on lance l'erreur comme dans votre code original.
    clang_disposeIndex(index);
    throw ZCError(ZC_FILE_PARSING_ERROR,
                  "Unable to parse translation unit: " + path_);
  }

  // 5. Lancer le visiteur
  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  VisitorContext ctx = {decls.get(), &content};
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

bool File::exists() const
{
  ifstream file(path_);
  return !file.fail();
}

bool File::writeDeclarations(const Declarations &decls) const
{
  stringstream content;
  string constant = getHeaderGuardConstant();

  // Custom header
  content << "/*\n\tThis file was automatically generated by ZC\n";
  auto now = chrono::system_clock::now();
  auto now_sec = chrono::floor<chrono::seconds>(now);
  string s = format("{:%F %T}", now_sec);
  content << "\tDate of creation: " << s << " (UTC)\n";
  content << "\tEditing this file manually could break it.\n*/\n\n";

  // Header guards
  content << "#ifndef " << constant << '\n';
  content << "#define " << constant << "\n\n";

  auto it = decls.find("includes");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Includes */\n";
    for (const auto &inc : it->second)
    {
      content << inc;
    }
    content << '\n';
  }

  it = decls.find("macros");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Macros */\n";
    for (const auto &macro : it->second)
    {
      content << "#define " << macro << '\n';
    }
    content << '\n';
  }

  content << '\n'
          << "#ifdef __cplusplus\n"
          << "extern \"C\" {\n"
          << "#endif\n\n";

  it = decls.find("enums");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Enums */\n";
    for (const auto &en : it->second)
    {
      content << en << ";\n";
    }
    content << '\n';
  }

  it = decls.find("unions");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Unions */\n";
    for (const auto &un : it->second)
    {
      content << un << ";\n";
    }
    content << '\n';
  }

  it = decls.find("structs");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Structures */\n";
    for (const auto &struc : it->second)
    {
      content << struc << ";\n";
    }
    content << '\n';
  }

  it = decls.find("typedefs");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Typedefs */\n";
    for (const auto &td : it->second)
    {
      content << td << ";\n";
    }
    content << '\n';
  }

  it = decls.find("globals");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Global variables */\n";
    for (const auto &glob : it->second)
    {
      content << glob << ";\n";
    }
    content << '\n';
  }

  it = decls.find("functions");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Functions */\n";
    for (const auto &func : it->second)
    {
      content << func << ";\n";
    }
    content << '\n';
  }

  content << "#ifdef __cplusplus\n"
          << "}\n"
          << "#endif\n"
          << "\n#endif // !" << constant << "\n";

  return writeContent(content.str());
}

string File::getExt() const { return fs::path(path_).extension().string(); }

void File::display(ostream &f) const
{
  f << "'";
  for (char c : path_)
  {
    if (c == '\'')
    {
      f << "'\\''";
    }
    else
    {
      f << c;
    }
  }
  f << "'";
}

ostream &operator<<(ostream &s, const File &f)
{
  f.display(s);
  return s;
}

bool File::containsMain() const
{
  CXIndex index = clang_createIndex(0, 0);

  // On peut utiliser des arguments vides ou les mêmes que pour parse()
  const char *args[] = {"-x", "c"};
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path_.c_str(), args, 2, nullptr, 0, CXTranslationUnit_None);

  if (unit == nullptr)
  {
    clang_disposeIndex(index);
    return false;
  }

  MainSearchContext context;
  CXCursor cursor = clang_getTranslationUnitCursor(unit);

  // Parcourir l'AST avec notre visiteur dédié
  clang_visitChildren(cursor, visitor_find_main, &context);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return context.found;
}

vector<string> File::getInclusions(const vector<Library> &libs) const
{
  vector<string> found_includes;
  vector<string> flags;

  CXIndex index = clang_createIndex(0, 0);

  // L'option DetailedPreprocessingRecord est OBLIGATOIRE pour voir les #include
  unsigned options = CXTranslationUnit_DetailedPreprocessingRecord;

  const char *args[] = {"-x", "c"};
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path_.c_str(), args, 2, nullptr, 0, options);

  if (unit)
  {
    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    // On récupère tous les noms de fichiers inclus
    clang_visitChildren(cursor, visitor_find_includes, &found_includes);

    // On compare avec votre map de bibliothèques pour extraire les flags
    for (const auto &inc : found_includes)
    {
      for (int i = 1; i < libs.size(); i++)
      {
        // Si l'inclusion contient le nom de la lib (ex: "math" dans "math.h")
        if (inc.find(libs[i].libname_) != string::npos)
        {
          flags.push_back(libs[i].compiling_option_);
        }
      }
    }

    clang_disposeTranslationUnit(unit);
  }

  clang_disposeIndex(index);
  return flags;
}

bool File::appendContent(const string &content) const
{
  ofstream file(path_, ios::app);

  if (!file.is_open())
    return false;

  file << content;
  return file.good();
}

string File::getHeaderGuardConstant() const
{
  return "_" + upper(split(getFilename(), '.').front()) + "_H_";
}

bool File::initPython() const
{
  stringstream content;
  content << "def main() -> None:\n"
          << "\tpass\n\n"
          << "if __name__ == '__main__':\n"
          << "\tmain()\n";

  return writeContent(content.str());
}

bool File::initC() const
{
  stringstream content;
  content << "#include <stdio.h>\n"
          << "#include <stdlib.h>\n\n"
          << "int main(void)\n"
          << "{\n"
          << "\treturn EXIT_SUCCESS;\n"
          << "}\n";

  return writeContent(content.str());
}

bool File::initCPP() const
{
  stringstream content;
  content << "#include <iostream>\n\n"
          << "using namespace std;\n\n"
          << "int main(void)\n"
          << "{\n"
          << "\treturn 0;\n"
          << "}\n";

  return writeContent(content.str());
}

bool File::initH() const
{
  string constant = getHeaderGuardConstant();
  stringstream content;
  content << "#ifndef " << constant << '\n'
          << "#define " << constant << "\n\n\n"
          << "#ifdef __cplusplus\n"
          << "extern \"C\" {\n"
          << "#endif\n\n\n"
          << "#ifdef __cplusplus\n"
          << "}\n"
          << "#endif\n\n"
          << "#endif // !" << constant << '\n';

  return writeContent(content.str());
}

bool File::initHPP() const
{
  stringstream content;
  content << "#pragma once\n\n";

  return writeContent(content.str());
}

bool File::copyTo(File &f) const
{
  string content = readFile();

  return f.writeContent(content);
}
