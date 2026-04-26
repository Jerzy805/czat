#pragma once
#include <string>
#include <vector>

void register_user(std::string nick, std::string id);
void unregister_user(std::string nick, std::string id);
std::vector<std::vector<std::string>> load_lobby();