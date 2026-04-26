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
    cin >> chat_name; // przydało by się tu dodać obsługę żeby nazwa konwersacji nie była pusta lub biała

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
            extract_args(line);
            add_user(added_nick, added_id);
            cout << "Pomyślnie dodano użytkownika " << added_nick << endl;
        }
        
        cout << "> ";
    }
}