// #ifndef CORE_H_
// #define CORE_H_
//
// // Includes
//
// #include <map>
// #include <nlohmann/json.hpp>
// #include <regex>
// #include <string>
// #include <vector>
//
// // Macros
//
// // clang-format off
// #if defined(_WIN32) || defined(_WIN64)
//   // On Windows, store config in the current directory or AppData
//   #define PATH "config.json"
// #elif defined(__APPLE__)
//   // Idk shit bout macOS
//   // js hope it follows Unix standards
//   #define PATH "/usr/local/etc/zc/config.json"
// #else
//   #define PATH "/etc/zc/config.json"
// #endif
// // clang-format on
//
// using json = nlohmann::json;
//
// // Utilities
//
// /**
//  * @brief Get the length of a string
//  */
// int len(const char *str);
// /**
//  * @brief Split a string by a delimiter.
//  */
// std::vector<std::string> split(const std::string &s, char delimiter);
//
// /**
//  * @brief Class used to perform multiple actions on files, such as parsing
//  */
// class FileParser
// {
// public:
//   explicit FileParser(const std::string &path);
//   /**
//    * @brief Get the inclusions from the file based on a map of libraries that
//    * require flags when compiling.
//    */
//   std::vector<std::string>
//   getInclusions(const std::map<std::string, std::string> &libs) const;
//   /**
//    * @brief Append content to the file.
//    */
//   bool appendContent(const std::string &content) const;
//
// private:
//   /**
//    * @brief Find all matches of a regex in a given text.
//    */
//   std::vector<std::string> findAll(const std::string &text,
//                                    const std::regex &re);
//   /**
//    * @brief Remove comments from the given text.
//    */
//   std::string removeComments(const std::string &text);
//   /**
//    * @grief Extract code blocks based on a keyword (e.g., struct, enum).
//    */
//   std::vector<std::string> extractBlock(const std::string &content,
//                                         const std::string &keyword);
//   /**
//    * @brief Path to the file being parsed.
//    */
//   std::string path_;
// };
//
// #endif // !CORE_H_
