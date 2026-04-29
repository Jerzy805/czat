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

void exit_cleanup()
{
    unregister_user(name);
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

    atexit(exit_cleanup);

    signal(SIGINT, cleanup);
    signal(SIGHUP, cleanup);

    system("clear");

    cout << "Podaj swój nick:\n";
    bool temp = false;

    while (!temp)
    {
        cin >> name;
        
        auto list = load_lobby();
        if (list.size() == 0)
        	break;

        for (int i = 0; i < list.size(); i++)
        {
            if (list[i][0] == name)
            {
                system("clear");
                cout << "Już istnieje użytkownik o tym nicku, podaj inny:\n";
                continue;
            }
            temp = true;
        }
    }

    register_user(name); // dodanie użytkownika do lobby

    //system("clear");
    int option;

    cout << "1. Nowy czat\n";
        cout << "2. Hostuj czat grupowy\n";
        cout << "3. Dołącz do czatu grupowego\n";
        cout << "Aby wysłać plik, wpisz !==!, a następnie nazwę pliku, np !==!video.mp4\n";
        cout << "Aby usunąć dany czat, wpisz jego numer z ujemnym znakiem\n";

        cin >> option;

        system("clear");

        if (option == 1) // nowy czat
        {
            string friend_name, friend_id;
            int choice = 0, j = 0;

            auto list = load_lobby(); // nie pokazuje użytkowników o tym samym id co my, także testy lokalne nie mają już sensu

            while (true)
            {
                system("clear");
                list = load_lobby();

                cout << "--- DOSTĘPNI UŻYTKOWNICY ---" << endl;
                for (j = 0; j < list.size(); j++) {
                    cout << j + 1 << ". " << list[j][0] << endl;
                }

                cout << list.size() + 1 << ". ODŚWIEŻ LISTĘ" << endl;
                
                cout << "\nWybierz opcję: ";
                cin >> choice;

                if (choice == list.size() + 1) // odświeża listę
                {
                    continue;
                }
                
                if (choice > 0 && choice < list.size() + 1) // wybór z lobby
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
                cout << "Błędna opcja!" << endl;
                return 1;
            }
        }
        else if (option == 2) // hostowanie czatu grupowego
        {
            auto list = load_lobby();
            vector<vector<string>> added_people;
            int choice = 0, actual_size, j;

            while (true)
            {
                system("clear");
                
                list = load_lobby();

                // Usuwamy z 'list' osoby, które są już w 'added_people'
                list = clear_list(added_people);

                actual_size = list.size();

                cout << "Wybierz opcję:\n" ;

                for (j = 0; j < actual_size; j++)
                {
                    cout << j + 1 << ". " << list[j][0] << endl;
                }
                cout << j + 1 << ". ODŚWIEŻ LISTĘ\n";
                cout << j + 2 << ". Zakończ dodawanie użytkowników\n";
                
                cin >> choice;

                if (choice == j + 1) // odświeżenie
                    continue;

                if (choice == j + 2) // zakończenie dodawania
                    break;

                added_people.push_back({list[choice - 1][0], list[choice - 1][1]});
                list.erase(list.begin() + choice - 1);
            }

            string final_nicks, final_ids;

            for (j = 0; j < added_people.size(); j++)
            {
                final_nicks += " " + added_people[j][0];
                final_ids += " " + added_people[j][1];
            }

            string cmd = "./grouphost " + name + final_nicks + final_ids;
            system(cmd.c_str());

        }
        else if (option == 3) // dołączanie do czatu grupowego
        {
            string group_chat_name;
            string prefix = "chat_group-";

            auto chats = get_group_chats();
            int choice, j;

            while (true)
            {
                system("clear");
                chats = get_group_chats();

                for (j = 0; j < chats.size(); j++)
                {
                    cout << j + 1 << ". " << chats[j] << endl;
                }
                cout << j + 1 << ". Odśwież\n";
                cin >> choice;

                if (choice == j + 1)
                {
                    continue;
                }
                if (choice > j + 1 || choice < 1)
                {
                    perror("Wtf typie\n");
                    return 1;
                }

                group_chat_name = chats[choice - 1];
                break;
            }

            execlp("./groupjoin", "groupjoin", name.c_str(), group_chat_name.c_str(), NULL);
        }
}
