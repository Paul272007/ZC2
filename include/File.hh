#ifndef FILE_H_
#define FILE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Config.hh>

using Declarations = std::map<std::string, std::vector<std::string>>;

class File
{
public:
  File(std::string filepath);
  ~File();

  /**
   * @brief Get the file path
   * @return path_
   */
  std::string getPath_() const;

  /**
   * @brief Get the file extension
   */
  std::string getExt() const;

  std::unique_ptr<Declarations> parse() const;

  /**
   * @brief Check if the file contains a main function.
   */
  bool containsMain() const;

  /**
   * @brief Get all #include in the file
   */
  std::vector<std::string>
  getInclusions(const std::vector<Library> &libs) const;

  /**
   * @brief Write content to the file.
   */
  bool writeContent(const std::string &content) const;

  /**
   * @brief Append content at the end of the file
   */
  bool appendContent(const std::string &content) const;

  /**
   * @brief Check if the file exists.
   */
  bool exists() const;

  /**
   * @brief Write the formatted C declarations to the file.
   */
  bool writeDeclarations(const Declarations &decls) const;

  /**
   * @brief Initialize a new python file
   */
  bool initPython() const;

  /**
   * @brief Initialize a new C source file
   */
  bool initC() const;

  /**
   * @brief Initialize a new C++ source file
   */
  bool initCPP() const;

  /**
   * @brief Initialize a new C header file
   */
  bool initH() const;

  /**
   * @brief Initialize a new C++ header file
   */
  bool initHPP() const;

  /**
   * @brief Display the filepath to f
   */
  void display(std::ostream &f = std::cout) const;

  /**
   * @brief Get the constant corresponding to this file to make header guards
   * @return FILENAME_H_
   */
  std::string getHeaderGuardConstant() const;

private:
  std::string path_;
  /**
   * @brief Reads the content of the file.
   */
  std::string readFile() const;
};

std::ostream &operator<<(std::ostream &s, const File &f);

#endif // !FILE_H_
