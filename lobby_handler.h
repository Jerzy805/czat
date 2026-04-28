#pragma once
#include <string>
#include <vector>

std::string get_my_id(); // zwraca id użytkownika w spk (login)
void register_user(std::string nick); // dodaje użytkownika do lobby
void unregister_user(std::string nick); // usuwa użytkownika z lobby
std::vector<std::vector<std::string>> load_lobby(); // wczytuje lobby z pominięciem użytkownika
std::vector<std::string> get_group_chats(); // zwraca istniejące czaty grupowe, do których użytkownik ma dostęp

std::vector<std::vector<std::string>> clear_list(const std::vector<std::vector<std::string>>& target_ids_vector);
// zwraca lobby, z pominięciem użytkowników, których id znajdują się już w argumencie funkcji
// uwaga: można rozważyć dodanie bardziej uniwersalnej funkcji która przyjmuje dowolne vectory stringów
// wymagałoby to jednak prawdopodobnie zmianny systemu zapisywania i odczytywania lobby, z nick(id) na id(nick)