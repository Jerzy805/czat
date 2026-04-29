	#include <string>
	#include <iostream>
	#include <vector>
	#include <fstream>
	#include <filesystem>
	#include <vector>
	#include <algorithm>
	#include <regex>

	using namespace std;
	namespace fs = filesystem;
	const string lobby = "/tmp/";

	string get_my_id()
	{
		const char* user = getenv("USER");

		if (user == nullptr)
		    return "unidentified ssh name";

		return (string)user;
	}

	void register_user(string nick)
	{
		string id = get_my_id();
		string text = "lobby_" + nick + "Q" + id;

		string cmd = "touch " + lobby + text;
		cout << cmd << endl;
		system(cmd.c_str());	
	}

void unregister_user(string nick)
{
	string id = get_my_id();
	string text = "lobby_" + nick + "Q" + id;
	string cmd = "rm " + lobby + text;
	system(cmd.c_str());
}

vector<vector<string>> load_lobby()
{
    vector<vector<string>> data;
    string path = lobby;
    
    // lobby_ - szuka stałego początku
    // ([^Q]+) - Grupa 1: nick (wszystko do napotkania litery Q)
    // Q       - separator
    // (.+)    - Grupa 2: ID (wszystko do końca nazwy pliku)
    regex pattern(R"(lobby_([^Q]+)Q(.+))");
    smatch matches;

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                string filename = entry.path().filename().string();

                if (regex_search(filename, matches, pattern) && matches.size() == 3) {
                    string nick = matches[1].str();
                    string id = matches[2].str();

                    // Pomijamy własne ID
                    if (id != get_my_id()) {
                        data.push_back({nick, id});
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Błąd systemu plików: " << e.what() << endl;
    }

    return data;
}

	vector<string> get_group_chats()
	{
		string prefix = "chat_group-";
		vector<string> chats;

		if (!fs::exists("/tmp")) return chats;

		for (const auto& entry : fs::directory_iterator("/tmp"))
		{
		    string filename = entry.path().filename().string();

		    if (filename.find(prefix) == 0) 
		    {
		        ifstream f(entry.path());

		        if (f.is_open()) // sprawdzenie czy mam uprawnienia by otwierać plik
		        {
		            //dodawanie do wektora samej nazwy czatu
		            chats.push_back(filename.substr(prefix.length()));
		            f.close();
		        }
		    }
		}

		return chats;
	}

	vector<vector<string>> clear_list(const vector<vector<string>>& target_ids_vector)
	{
		auto list = load_lobby();

		list.erase(
		    remove_if(list.begin(), list.end(), [&](const vector<string>& row_in_lobby) {
		        // sprawdza czy id z lobby istnieje w targetowanym vectorze
		        return any_of(target_ids_vector.begin(), target_ids_vector.end(), [&](const vector<string>& temp_vector) {
		            return temp_vector[1] == row_in_lobby[1]; 
		        });
		    }), 
		    list.end()
		);

		return list;
	}
