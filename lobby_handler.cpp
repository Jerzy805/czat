#include <string>
#include <vector>
#include <fstream>
#include <vector>
#include <regex>

using namespace std;
const string lobby = "/tmp/lobby";

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
    string text = nick + "(" + id + ")\n";

    // szukanie textu w lobby, żeby nie pisać dwa razy
    ifstream file(lobby);
    string file_content;

    if (file.is_open())
    {
        getline(file, file_content, '\0');
        file.close();
    }

    if (file_content.find(text) != string::npos)
        return;

    // dopisywanie użytkownika do lobby
    ofstream f(lobby, ios::app);
    
    if (f.is_open())
    {
        f << text;
        f.close();
    }
    else
    {
        perror("Lobby - ofstream");
    }
}

void unregister_user(string nick)
{
    string id = get_my_id();
    string target = nick + "(" + id + ")";
    vector<string> lines;
    string line;

    // Odczytujemy cały plik do pamięci
    ifstream inFile(lobby);
    if (inFile.is_open()) {
        while (getline(inFile, line)) {
            // Dodajemy linię do wektora tylko jeśli nie jest tą, którą chcemy usunąć
            if (line != target) {
                lines.push_back(line);
            }
        }
        inFile.close();
    }

    // Nadpisujemy plik nową zawartością (bez usuniętej linii)
    ofstream outFile(lobby, ios::trunc);
    for (const auto& l : lines) {
        outFile << l << endl;
    }
}

vector<vector<string>> load_lobby()
{
    vector<vector<string>> data;
    ifstream file(lobby);
    string line;
    
    // Regex do wyciągania nick i id z formatu nick(id)
    regex pattern(R"((.+)\((.+)\))");
    smatch matches;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (regex_search(line, matches, pattern) && matches.size() == 3) {
                // matches[1] to string1, matches[2] to string2
                data.push_back({matches[1].str(), matches[2].str()});
            }
        }
        file.close();
    }
    return data;
}