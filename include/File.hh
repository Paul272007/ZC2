#ifndef FILE_H_
#define FILE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
  getInclusions(const std::map<std::string, std::string> &libs) const;

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
   * @brief Write the formatted declarations to the file.
   */
  bool writeDeclarations(const Declarations &decls,
                         const std::string &constant) const;
  /**
   * @brief Display the filepath to f
   */
  void display(std::ostream &f = std::cout) const;

private:
  std::string path_;
  /**
   * @brief Reads the content of the file.
   */
  std::string readFile() const;
};

std::ostream &operator<<(std::ostream &s, const File &f);

#endif // !FILE_H_
