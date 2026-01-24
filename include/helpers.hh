#include <filesystem>
#include <string>

#define ROOT_DIR ".zc"

/**
 * @brief Returns the root directory of ZC
 */
std::filesystem::path get_zc_path();

std::string escape_shell_arg(const std::string &arg);
