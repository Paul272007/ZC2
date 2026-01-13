#ifndef HELPERS_H_
#define HELPERS_H_

#include <string>
#include <vector>

// using json = nlohmann::json;

/**
 * @brief Ask the user for a yes/no confirmation.
 *
 * @return true if the user answered yes, false otherwise.
 * Return valued should not be discarded.
 */
[[nodiscard]] bool ask(const std::string &question);

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

std::vector<std::string> split(const std::string &s, char delimiter);

#endif // !HELPERS_H_
