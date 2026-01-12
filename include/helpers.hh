#ifndef HELPERS_H_
#define HELPERS_H_

#include <string>

using namespace std;
// using json = nlohmann::json;

/**
 * @brief Ask the user for a yes/no confirmation.
 * @return true if the user answered yes, false otherwise.
 */
[[nodiscard]] bool ask();

/**
 * @brief Format and print a success message.
 */
void success(const std::string &msg);

/**
 * @brief Format and print a debugging message.
 */
void debug(const std::string &msg);

/**
 * @brief Format and print a warning.
 */
void warning(const std::string &msg);

/**
 * @brief Format and print an info.
 */
void info(const std::string &msg);

/**
 * @brief Convert a string to uppercase.
 */
std::string upper(std::string str);

/**
 * @brief Escapes a string to be safely used as a shell argument.
 */
std::string escape_shell_arg(const std::string &arg);

#endif // !HELPERS_H_
