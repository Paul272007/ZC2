#include <iostream>
#include <string>

#include <display.h>

using namespace std;

bool ask()
{
  string line;
  cout << "[Y/n] ";

  while (getline(cin, line))
  {
    // 1. If the line is empty, we consider it as 'yes'
    if (line.empty())
    {
      return true;
    }

    // 2. Otherwise, we check the first character
    char input = toupper(line[0]);

    if (input == 'Y')
      return true;
    if (input == 'N')
      return false;

    cout << "Error: unexpected token" << endl << "[Y/n] ";
  }

  return true; // Security if the input stream is closed
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

string upper(string str)
{
  for (char &c : str)
  {
    c = toupper(c);
  }
  return str;
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
