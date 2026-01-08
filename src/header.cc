#include "core.hpp"
#include <header.hpp>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

int header_create(vector<string> files, bool force, string output)
{
  // debug("header_create called");
  if (files.size() == 0)
  {
    throw ZCError(5, "No input files provided.");
  }
  if (output.empty())
  {
    output = split(files[0], '.').front() + ".h";
  }
  FileParser output_parser(output);
  if (output_parser.exists() && !force)
  {
    cout << "The file " << output
         << " already exists. Do you want to overwrite it?" << endl;
    if (!ask())
    {
      throw ZCError(6, "Operation cancelled by user.");
    }
  }
  stringstream constant;
  string filename = split(output, '/').back();
  string without_ext = split(filename, '.').front();
  constant << "_" << upper(without_ext) << "_H_";
  Declarations all_decls;

  auto merge_decls = [&](const vector<string> &src, vector<string> &dest)
  {
    std::unordered_set<std::string> existing(dest.begin(), dest.end());

    for (const auto &item : src)
    {
      if (existing.find(item) == existing.end())
      {
        dest.push_back(item);
        existing.insert(
            item); // Optionnel : si 'src' peut lui-même avoir des doublons
      }
    }
  };

  for (const auto &file : files)
  {
    FileParser parser(file);
    Declarations decls = parser.parse();
    merge_decls(decls.includes, all_decls.includes);
    merge_decls(decls.macros, all_decls.macros);
    merge_decls(decls.globals, all_decls.globals);
    merge_decls(decls.typedefs, all_decls.typedefs);
    merge_decls(decls.enums, all_decls.enums);
    merge_decls(decls.functions, all_decls.functions);
    merge_decls(decls.structs, all_decls.structs);
    merge_decls(decls.unions, all_decls.unions);
  }
  return output_parser.writeDeclarations(all_decls, constant.str());
}

int header_sync(string h, string c, string output)
{
  debug("header_sync called");
  return 0;
}

int header_init(string h, bool force, string output)
{
  // debug("header_init called");
  FileParser parser(h);
  if (!force && parser.exists())
  {
    cout << "The file " << h << " already exists. Do you want to overwrite it?"
         << endl;
    bool ans = ask();
    if (!ans)
      throw ZCError(6, "Operation cancelled by user.");
  }
  stringstream constant;
  string filename = split(h, '/').back();
  string without_ext = split(filename, '.').front();
  constant << "_" << upper(without_ext) << "_H_";

  stringstream content;
  content << "#ifndef " << constant.str() << '\n';
  content << "#define " << constant.str() << "\n\n\n";
  content << "#endif // !" << constant.str() << '\n';

  if (!parser.writeContent(content.str()))
  {
    throw ZCError(7, "Could not write to file " + h);
  }
  return 0;
}
