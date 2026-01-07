#include <header.hpp>
#include <iostream>

using namespace std;

int header_create(vector<string> files, bool force, string output)
{
  debug("header_create called");
  return 0;
}
int header_sync(string h, string c, string output)
{
  debug("header_sync called");
  return 0;
}
int header_init(string h, bool force, string output)
{
  debug("header_init called");
  FileParser parser(h);
  if (!force && parser.exists())
  {
    cout << "The file" << h << "already exists. Do you want to overwrite it?"
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
  content << "#endif // !" << constant.str() << "\n";

  if (!parser.writeContent(content.str()))
  {
    throw ZCError(7, "Could not write to file " + h);
  }
  return 0;
}
