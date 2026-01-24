#include <filesystem>
#include <string>
#include <vector>

#define ROOT_DIR ".zc"

std::string escape_shell_arg(const std::string &arg);

std::vector<std::string> split(const std::string &s, char delimiter);

/**
 * @brief Returns the root directory of ZC
 */
std::filesystem::path getZCRootDir();

std::string join(const std::vector<std::string> &v,
                 const std::string &separator);
