#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <array>
#include <memory>
#include <algorithm>
#include "lobby_handler.h"

using namespace std;
namespace fs = filesystem;
using namespace fs;

string name;
string found_friends[20];
int friends_count; // liczba znalezionych przyjaciół

void cleanup(int signum)
{
    // tutaj obsługa wywalania użytkowników ze wspólnego pliku "lobby"
    unregister_user(name);
    exit(0); // bardzo ważne XDDD
}

int get_existing() // pobranie istniejących konwersacji
{
    string prefix = "chat_" + name + "-";
    int counter = 0; // zwracana wartość, liczba znalezionych konwersacji
    
    // szukanie plików po prefixie

    for (const auto& entry : directory_iterator("/tmp/"))
    {
        string filename = entry.path().filename().string();

        if (filename.rfind(prefix, 0) == 0 && filename.find('!') == string::npos)
        {
            string found_friend = filename.substr(prefix.length());
            found_friends[counter++] = found_friend;

            if (counter >= 20)
                return counter;
        }
    }
    
    if (counter == 0)
    {
        cout << "Nie znaleziono żadnych zapisów konwersacji\n";
    }
    
    return counter;
}

string get_id(const string& filename) // pobiera id hosta rozmowy
{
    string cmd = "getfacl " + filename;

    array<char, 256> buffer;
    string result;

    // popen w wersji "bezpiecznej" (RAII)
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        perror("popen");
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        string line = buffer.data();

        if (line.rfind("user:", 0) == 0 && line.find("::") == string::npos)
        {
            // wyciągnięcie id
            size_t start = 5; // po "user:"
            size_t end = line.find(':', start);

            if (end != string::npos)
            {
                return line.substr(start, end - start);
            }
        }
    }

    return "";
}

int delete_convo(int index)
{
    if(index >= friends_count)
    {
        return 1;
    }

    string filename = "/tmp/chat_" + name + "-" + found_friends[index];

    if (remove(filename.c_str()) == 0)
    {
        return 0;
    }
    return 1;
}

int create_connection(string friend_name, string id) // tworzy plik i nadaje uprawnienia
{
    string filename = "/tmp/chat_" + name + "-" + friend_name;

    ofstream f(filename);
    f.close();

    // odebranie wszelkich praw wszystkim
    if (chmod(filename.c_str(), 0600) == -1)
    {
        perror("chmod");
        return 1;
    }

    string cmd = "setfacl -m u:" + id + ":r " + filename;
    
    if (system(cmd.c_str()) == -1)
    {
        perror("system");
        return 1;
    }

    return 0;
}

int main()
{
    setlocale(LC_ALL, "");
    // utworzenie pliku lobby odbywa się w register_user

    signal(SIGINT, cleanup);

    system("clear");
    cout << "Podaj swój nick:\n";
    cin >> name;

    register_user(name);

    friends_count = get_existing();

    // tworzenie menu

    system("clear");
    int i = 0, option;

    if (friends_count > 0)
    {
        for (i = 0; i < friends_count; i++)
        cout << i + 1 << ". " << found_friends[i] << endl;
    }    

        cout << i + 1 << ". Nowy czat\n";
        cout << i + 2 << ". Hostuj czat grupowy\n";
        cout << i + 3 << ". Dołącz do czatu grupowego\n";
        cout << "Aby wysłać plik, wpisz !==!, a następnie nazwę pliku, np !==!video.mp4\n";
        cout << "Aby usunąć dany czat, wpisz jego numer z ujemnym znakiem\n";

        cin >> option;

        system("clear");

        if (option == i + 1) // nowy czat
        {
            string friend_name, friend_id;
            int choice = 0, j = 0;

            auto list = load_lobby();

            while (true)
            {
                system("clear"); // Czyścimy ekran przy każdym odświeżeniu
                list = load_lobby();
                list.erase(remove_if(list.begin(), list.end(), [&](const vector<string>& row) {
                    return row[0] == name;
                }), list.end());

                cout << "--- DOSTĘPNI UŻYTKOWNICY ---" << endl;
                for (j = 0; j < list.size(); j++) {
                    cout << j + 1 << ". " << list[j][0] << endl;
                }

                cout << list.size() + 1 << ". ODŚWIEŻ LISTĘ" << endl;
                cout << list.size() + 2 << ". Wprowadź dane ręcznie" << endl;
                
                cout << "\nWybierz opcję: ";
                cin >> choice;

                if (choice == list.size() + 1) // odświeża listę
                {
                    continue;
                }
                
                if (choice > 0 && choice <= list.size() + 2) // ręczne wprowadzanie danych lub wybór z lobby
                {
                    break;
                }
                
                cout << "Niepoprawny wybór!" << endl;
                sleep(1); // Krótka pauza przed odświeżeniem po błędzie
            }

            if (choice != j + 1) // wybór z lobby
            {
                friend_name = list[choice - 1][0];
                friend_id = list[choice - 1][1];

                create_connection(friend_name, friend_id);

                execlp("./chat", "chat", name.c_str(), friend_name.c_str(), friend_id.c_str(), NULL);
            }
            else // ręczne wprowadzanie danych
            {
                cout << "Podaj nick rozmówcy:\n";
                cin >> friend_name;
                cout << "Podaj jego nazwę w spk:\n";
                cin >> friend_id;

                create_connection(friend_name, friend_id);

                execlp("./chat", "chat", name.c_str(), friend_name.c_str(), friend_id.c_str(), NULL);
            }
        }
        else if (option == i + 2) // hostowanie czatu grupowego
        {
            auto list = load_lobby();
            vector<vector<string>> added_people;
            int choice = 0, actual_size, j;

            do
            {
                system("clear");
                
                list = load_lobby();
                // usuwanie użytkownika z listy
                list.erase(remove_if(list.begin(), list.end(), [&](const vector<string>& row)
                {
                        return row[0] == name;

                }), list.end());

                // Usuwamy z 'list' osoby, które są już w 'added_people'
                list.erase(std::remove_if(list.begin(), list.end(), [&](const vector<string>& row_in_lobby) {
                    // Sprawdzamy, czy nick z lobby (row_in_lobby[0]) istnieje w added_people
                    return std::any_of(added_people.begin(), added_people.end(), [&](const vector<string>& already_added) {
                        return already_added[0] == row_in_lobby[0]; 
                    });
                }), list.end());

                actual_size = list.size();

                cout << "Wybierz opcję:\n" ;

                for (j = 0; j < actual_size; j++)
                {
                    cout << j + 1 << ". " << list[j][0] << endl;
                }
                cout << j + 1 << ". ODŚWIEŻ LISTĘ\n";
                cout << j + 2 << ". Zakończ dodawanie użytkowników\n";
                
                cin >> choice;

                if (choice == j + 1)
                    continue;

                if (choice == j + 2)
                    break;

                added_people.push_back({list[choice - 1][0], list[choice - 1][1]});
                list.erase(list.begin() + choice - 1);
            } while (list.size() != 0);

            string final_nicks, final_ids;

            for (j = 0; j < added_people.size(); j++)
            {
                final_nicks += " " + added_people[j][0];
                final_ids += " " + added_people[j][1];
            }

            string cmd = "./grouphost " + name + final_nicks + final_ids;
            system(cmd.c_str());

        }
        else if (option == i + 3) // dołączanie do czatu grupowego
        {
            string group_chat_name, fgroup_chat_name;
            bool is_ok;

            do
            {
                cout << "Podaj nazwę czatu:\n";
                cin >> group_chat_name;

                fgroup_chat_name = "/tmp/chat_group-" + group_chat_name;
                bool if_exists, is_given;

                // sprawdzenie czy czat istnieje
                if (!exists(fgroup_chat_name))
                {
                    cout << "Nie istnieje taki czat!\n";
                    if_exists = false;
                }
                else 
                    if_exists = true;

                // sprawdzenie czy mamy do niego uprawnienia
                if (access(fgroup_chat_name.c_str(), R_OK) != 0)
                {
                    cout << "Host nie nadał ci uprawnień do tej konwersacji\n";
                    is_given = false;
                }
                else
                    is_given = true;

                is_ok = is_given && if_exists;
            } while (!is_ok);

            execlp("./groupjoin", "groupjoin", name.c_str(), group_chat_name.c_str(), NULL);
        }
        else if (option <= i && option > 0) // otwieranie istniejącej konwersacji
        {
            string filename = "/tmp/chat_" + name + "-" + found_friends[option - 1];

            execlp("./chat", "chat", name.c_str(), found_friends[option - 1].c_str(), get_id(filename).c_str(), NULL);
        }
        else if (option < 0) // usuwanie konwersacji
        {
            if (delete_convo(-option - 1) == 0)
            {
                cout << "Pomyślnie usunięto konwersację z " << found_friends[-option - 1] << endl;
                friends_count = get_existing();
            }
            else
            {
                if (option == -2137) // usuwanie wszystkich istniejących konwersacji
                {
                    system("rm /tmp/chat* -f");
                    cout << "Pomyślnie usunięto wszystkie konwersacje\n";
                    return 0;
                }
                cout << "Niepoprawna komenda\n";
                return 1;
            }
        }
}