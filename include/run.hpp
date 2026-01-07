#ifndef RUN_H_
#define RUN_H_

#include <algorithm>
#include <core.hpp>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

int run_func(const std::vector<std::string> &files, bool keep, bool plus,
             bool compile_only, const std::vector<std::string> &args);

#endif // !RUN_H_
