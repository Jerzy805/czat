#pragma once
#include <string>
#include <vector>

std::string get_my_id();
void register_user(std::string nick);
void unregister_user(std::string nick);
std::vector<std::vector<std::string>> load_lobby();
