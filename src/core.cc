// #include <ZCError.hh>
// #include <clang-c/Index.h>
// #include <display.h>
// #include <string>
// #include <vector>
//
// using namespace std;
//
// // ===================================================== Helpers
//
// int len(const char *str)
// {
//   int i = 0;
//   while (*str++)
//   {
//     i++;
//   }
//   return i;
// }
//
// // ================================== Functions that will get exported
//
// vector<string> split(const string &s, char delimiter)
// {
//   vector<string> tokens;
//   string token;
//   istringstream tokenStream(s);
//   while (getline(tokenStream, token, delimiter))
//   {
//     tokens.push_back(token);
//   }
//   return tokens;
// }
