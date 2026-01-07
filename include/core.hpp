#ifndef CORE_H_
#define CORE_H_

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <zcerror.hpp>

#define PATH "../etc/config.json"

using json = nlohmann::json;

json get_conf();
void write_conf(json conf);
[[nodiscard]] bool ask();

#endif // !CORE_H_
