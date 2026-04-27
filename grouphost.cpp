#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <sys/inotify.h>
#include <algorithm>
#include <unordered_set>
#include "lobby_handler.h"

using namespace std;
namespace fs = filesystem;

int users_count;
vector<string> nicks;
vector<string> ids;
string chat_name;
string main_file;
string name;
string added_nick, added_id;

void append_text(string text, string user)
{
    ofstream f(main_file, ios::app);
    if (!f.is_open())
    {
        perror("ofstream");
        return;
    }

    if (text.find("[System]") == string::npos)
    {
        f << "[" << user << "] ";
    }

    f << text << endl;
    f.close();
}

void cleanup(int singum)
{
    unregister_user(name); // wypisanie z lobby
    append_text("Gospodarz wyszedł ze spotkania", "System");
    system("rm /tmp/chat* -f");
    exit(0);
}

void clear_list(vector<vector<string>>& target_list) // usuwa z listy te elementy, które są już w ids
{
    // Tworzymy set na bazie globalnego (lub przekazanego) ids
    unordered_set<string> s(ids.begin(), ids.end());

    // Pracujemy na referencji target_list
    target_list.erase(
        remove_if(target_list.begin(), target_list.end(), [&](const vector<string>& row) {
            return s.count(row[1]);
        }), 
        target_list.end()
    );
}

bool check_line(string line)
{
    return line.find("!=");
}

void reader(string user) 
{
    string filename = main_file + "-" + user;

    ifstream f;

    // Czekamy aż plik się pojawi na dysku
    while (true)
    {
        f.open(filename);
        if (f.is_open())
            break;

        sleep(1);
    }

    // Przeskakujemy na koniec pliku, aby nie czytać starych wiadomości
    f.seekg(0, ios::end);

    // Inicjalizacja inotify do monitorowania zmian w pliku
    int fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        return;
    }

    int wd = inotify_add_watch(fd, filename.c_str(), IN_MODIFY);
    if (wd == -1)
    {
        perror("inotify_add_watch");
        close(fd);
        return;
    }

    char buffer[4096];

    while (true)
    {
        // Blokujemy się tutaj, dopóki w pliku nie pojawi się zmiana
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0)
        {
            perror("read");
            break;
        }

        // Czytamy wszystkie nowe linie, które dopisano do pliku
        string line;
        while (getline(f, line))
        {
            //writer(line); // Logika zapisu do logów/historii

            // if (check_msg(line))
            // {
            //     if (!file_handler())
            //     {
            //         printf("Nie przyjąłeś pliku\n");
            //         append_text("[System] Plik: nie przyjęto");
            //     }
            //     else
            //     {
            //         printf("Pomyślnie przyjąłeś plik\n");
            //         append_text("[System] Plik: pomyślnie przyjęto plik");
            //     }
            // }

            // Odświeżenie widoku w konsoli
            cout << "\r\033[K";        // Czyści bieżącą linię w terminalu
            cout << "[" << user << "] " <<line << endl;      // Wypisuje nową wiadomość
            cout << "> " << flush;     // Przywraca znak zachęty
            append_text(line, user); // utrwala wiadomośc w pliku głównym, żeby każdy miał do niej dostęp
        }

        // Czyścimy flagę błędu (EOF), żeby getline mógł czytać dalej po następnej zmianie
        f.clear();
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

int create_connection(string id) // tworzy plik i nadaje uprawnienia
{
    string cmd;

    // utworzenie pliku głównego jeżeli jeszcze nie istnieje
    if (!fs::exists(main_file))
    {
        cmd = "touch " + main_file;
        system(cmd.c_str());
    }

    // odebranie wszelkich praw wszystkim
    if (chmod(main_file.c_str(), 0600) == -1)
    {
        perror("chmod");
        return 1;
    }

    cmd = "setfacl -m u:" + id + ":r " + main_file;
    
    if (system(cmd.c_str()) == -1)
    {
        perror("system");
        return 1;
    }

    return 0;
}

void add_user(string user, string id) // funkcja do dynamicznego dodawania użytkowników do czatu
{
    create_connection(id);
    ids.push_back(id);

    thread t(reader, user);
    t.detach();
}

void extract_args(string line)
{
    size_t sep_pos = line.find("!= ");
    if (sep_pos == string::npos) return;

    size_t start_arg1 = sep_pos + 3;
    size_t comma_pos = line.find(",", start_arg1);
    if (comma_pos == string::npos) return;

    added_nick = line.substr(start_arg1, comma_pos - start_arg1);
    added_id = line.substr(comma_pos + 2);
}

bool is_str_empty(string line)
{
    if (line.empty())
        return true;
    for (char c : line)
    {
        if (!isspace(c))
            return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc % 2 != 0)
    {
        cout << "Błędna liczba argumentów!\n";
        return 1;
    }

    signal(SIGINT, cleanup);

    name = argv[1];

    cout << "Podaj nazwę konwersacji grupowej:\n";
    //cin >> chat_name; // przydało by się tu dodać obsługę żeby nazwa konwersacji nie była pusta lub biała
        
    bool correctChatName = false;

    while(!correctChatName){
        cout << "Podaj nazwę konwersacji grupowej:\n";
        cin >> chat_name;

        if(chat_name.empty() || chat_name[0] == ' '){
            cout << "Nazwa konwersacji nie może być pusta lub zawierać tylko białe znaki. Spróbuj ponownie.\n";
        } else {
            correctChatName = true;
        }
    }

    main_file = "/tmp/chat_group-" + chat_name;

    string cmd = "touch " + main_file;
    system(cmd.c_str());

    users_count = (argc - 2) / 2;

    ids.resize(users_count);
    nicks.resize(users_count);

    // wczytanie id użytkowników
    for (int i = 0; i < users_count; i++)
    {
        ids[i] = argv[users_count + 2 + i];
        // nadanie praw czytania użytkownikom
        create_connection(ids[i]);
    }

    // wczytanie nicków użytkowników i rozpoczęcie czytania ich wiadomości
    for (int i = 0; i < users_count; i++)
    {
        nicks[i] = argv[2 + i];
        // wrzucenie na osobne wątki czytania poszczególnych plików
        string filename = main_file + "-" + nicks[i];
        thread t(reader, nicks[i]);
        t.detach(); // odłączenie wątku t od głównego wątku, bez tego nie ma prawa działać
    }

    system("clear");
    cout << "---------Rozpoczęto konwersację grupową---------\n";

    string line;
    while (getline(cin, line))
    {
        if (check_line(line))
        {
            if (!is_str_empty(line))
            append_text(line, name);
        }
        else
        {
            auto list = load_lobby();

            unordered_set<string> s(ids.begin(), ids.end()); // zmienna tymczasowa

            while (true)
            {
                clear_list(list); // oczyszcza listę o dodanych już użtytkowników
                system("clear");
                if (list.size() == 0)
                {
                    cout << "Nie ma żadnych niedodanych użytkowników online\n";
                    break;
                }

                int i, choice;

                for (i = 0; i < list.size(); i++)
                {
                    cout << i + 1 << ". " << list[i][0] << endl; 
                }
                cout << i + 1 << ". Odśwież listę\n";
                cout << i + 2 << ". Zakończ dodawanie użytkowników\n";
                cin >> choice;

                if (choice == i + 1)
                    continue;
                if (choice == i + 2)
                    break;

                add_user(list[choice - 1][0], list[choice - 1][1]);
                cout << "Pomyślnie dodano użytkownika " << list[choice - 1][0] << endl;
                cout << "Naciśnij dowolny klawisz aby kontynuować\n";
                cin.get();
            }
            
        }
        
        cout << "> ";
    }
}