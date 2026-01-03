#ifndef LIB_H_
#define LIB_H_

#include <iostream>
#include <string>
#include <vector>

int lib_list();
int lib_install(std::string h, std::string a, bool force);
int lib_create(std::vector<std::string> files, bool force);
int lib_remove(std::string lib);

#endif // !LIB_H_
