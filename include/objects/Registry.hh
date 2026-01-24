#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <zcio.hh>

#include <objects/File.hh>

#define REGISTRY "zc-lock.json"

struct Package
{
  std::string name_;
  std::string author_;
  std::string version_;

  std::vector<std::string> headers_;
  std::vector<std::string> binaries_;

  std::string flags_;
};

class Registry
{
public:
  static Registry &getInstance();

  /**
   * @brief Load the Registry
   */
  void load();

  /**
   * @brief Save a library based on its configuration and its files (headers /
   * object files)
   * 1. Create a subdirectory in the ZC include dir
   * 2. Copy the header files into this directory
   * 3. Compile the object files into a static / dynamic library
   * 4. Put the binaries into the lib directory
   *
   * @param package The package configuration
   * @param headers The library's header files
   * @param objects The library's object files
   */
  void savePackage(Package &package, bool force,
                   std::vector<std::filesystem::path> &headers,
                   std::vector<std::filesystem::path> &objects,
                   std::vector<std::filesystem::path> &sources);

  std::vector<Package> getPackages() const;

  std::filesystem::path getIncludeDir() const;

  std::filesystem::path getLibDir() const;

  Table packagesTable() const;

private:
  Registry() = default;

  /**
   * @brief Index the Package
   */
  void indexPackage(const Package &package);

  void compileObjects(const std::vector<std::filesystem::path> &sources,
                      std::vector<std::filesystem::path> &objects) const;

  bool createStaticLib(const std::string &libPath,
                       const std::vector<std::filesystem::path> &objects) const;

  bool createSharedLib(const std::string &libPath,
                       const std::vector<std::filesystem::path> &objects) const;

  // void saveHeader(const File &header);
  // void saveBinary(const File &binary);

  std::filesystem::path include_path_;
  std::filesystem::path lib_path_;

  std::vector<Package> std_packages_;
  std::vector<Package> packages_;
};
