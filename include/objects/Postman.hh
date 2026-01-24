#pragma once

#include <string>
#include <vector>

#include <objects/File.hh>
#include <objects/Registry.hh>

class Postman
{
public:
  static Postman &getInstance();

  bool uploadPackage(const Package &package, const std::vector<File> &headers,
                     const std::vector<File> &binaries);

  bool downloadPackage(const std::string &target);

private:
  Postman() = default;
};
