#ifndef HEADER_H_
#define HEADER_H_

#include <core.hpp>
#include <string>
#include <vector>

int header_create(std::vector<std::string> files, bool force,
                  std::string output);
int header_sync(std::string h, std::string c, std::string output);
int header_init(std::string h, bool force, std::string output);

#endif // !HEADER_H_
