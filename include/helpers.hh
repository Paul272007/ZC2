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

/**
 * @brief Join all vector elements with the separator
 *
 * @param v The vector to be joined
 * @param separator The separator between each element
 */
std::string join(const std::vector<std::string> &v,
                 const std::string &separator);

/**
 * @brief Convert string to uppercase
 *
 * @param s The string to be converted
 */
std::string upper(const std::string &s);
