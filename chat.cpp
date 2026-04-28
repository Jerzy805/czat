#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <csignal>
#include <thread>
#include <filesystem>
#include <cstring>
#include "send_file.h"
#include "get_file.h"
#include "lobby_handler.h"

using namespace std;
namespace fs = filesystem;

string my_file, friend_file, file_to_send, sent_file, friend_name, name, id;
const string send_file_signal = "!==!";

void append_text(const string& text)
{
    string prefix = "[" + name + "] ";
    ofstream f(my_file, ios::app);
    if (!f.is_open())
    {
        perror("ofstream");
        return;
    }

    f << prefix << text << endl;
    f.close();
}

void cleanup(int signum)
{
    // tutaj obsługa wywalania użytkowników ze wspólnego pliku "lobby"
    unregister_user(name);
    append_text("[System] Konwersacja zakończona, użytkownik wyszedł");
    system("rm /tmp/chat* -f"); // usunięcie plików konwersacji, właściwie to nie są już potrzebne
    exit(0);
}

void exit_cleanup()
{
    unregister_user(name);
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

void writer(string line)
{

}

bool check_msg(string msg)
{
    size_t pos = msg.find(send_file_signal);
    if (pos != string::npos)
    {
        sent_file = msg.substr(pos + send_file_signal.length());
        return true;
    }
        
    return false;
}

bool file_handler()
{
    cout << "[System] " << friend_name << " chce ci wysłać plik " << sent_file << ", kontynuować [y/n]?\n";
    cout.flush();
    string answer;
    cin >> answer;

    if (answer[0] == 'y')
    {
        cout << "Rozpoczęto pobieranie pliku...\n";
        get_file(sent_file);
        return true;
    }
    return false;
}

void* reader(void* arg) // najważniejsza funkcja, odczytuje wiadomości drugiego rozmówcy
{
    // ?? czekaj aż plik się pojawi
    ifstream f;
    while (true)
    {
        f.open(friend_file);
        if (f.is_open())
            break;

        sleep(1);
    }

    // ?? ustaw się na koniec (jak tail -f)
    f.seekg(0, ios::end);

    // ?? inotify init
    int fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        return nullptr;
    }

    int wd = inotify_add_watch(fd, friend_file.c_str(), IN_MODIFY);
    if (wd == -1)
    {
        perror("inotify_add_watch");
        close(fd);
        return nullptr;
    }

    char buffer[4096];

    while (true)
    {
        // ? czekaj na zmianę w pliku
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0)
        {
            perror("read");
            break;
        }

        // ?? po zmianie — czytaj nowe linie
        string line;
        while (getline(f, line))
        {
            writer(line);

            if (check_msg(line))
            {
                if (!file_handler())
                {
                    printf("Nie przyjąłeś pliku\n");
                    append_text("[System] Plik: nie przyjęto");
                }
                else
                {
                    printf("Pomyślnie przyjąłeś plik\n");
                    append_text("[System] Plik: pomyślnie przyjęto plik");
                }
            }

            if (line.find("[System] Plik:") != string::npos)
            {
                string command = "rm /tmp/" + sent_file + " -f";
                system(command.c_str());
            }

            cout << "\r\033[K";        // usuń prompt (jeśli jest)
            cout << line << endl;     // wypisz wiadomość
            cout << "> " << flush;    // przywróć prompt

        }

        // ?? reset EOF flagi (bo getline dobił do końca)
        f.clear();
    }

    // cleanup
    inotify_rm_watch(fd, wd);
    close(fd);

    return nullptr;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Błąd krytyczny: nieprawidłowa liczba argumentów programu " << argv[0] << endl;
        return 1;
    }

    signal(SIGINT, cleanup);
    signal(SIGHUP, cleanup);

    atexit(exit_cleanup);

    name = argv[1];
    friend_name = argv[2];
    id = argv[3];

    register_user(name); // jeżeli użytkownik jest już w lobby to nie stanie się nic

    string prefix = "/tmp/chat_";
    my_file = prefix + name + "-" + friend_name;
    friend_file = prefix + friend_name + "-" + name;

    cout << "> ";

    thread t(reader, nullptr);

    system("clear");
    cout << "Rozpoczęto rozmowę z użytkownikiem " << friend_name << endl;

    string line;
    while (getline(cin, line))
    {
        if (check_msg(line))
        {
            // sprawdzenie, czy taki plik w ogóle istnieje
            if (!(fs::exists(sent_file)))
            {
                cout << "Podany plik nie istnieje!\n";
                line = "";
            }
            else
                send_file(sent_file, id);
        }
        if (!is_str_empty(line))
            append_text(line);
        cout << "> ";
    }

    t.join();
}
