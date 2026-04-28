#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <csignal>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <pwd.h>
#include <cstdlib>
#include <unistd.h>
#include "lobby_handler.h"

using namespace std;
namespace fs = filesystem;

string name;
string main_file;
string my_file;

void append_text(string text)
{
    ofstream f(my_file, ios::app);

    if (!f.is_open())
    {
        perror("ofstream (append_text -> groupjoin)");
        return;
    }

    f << text << endl; // prefix niepotrzebny, bo groupjoin wie z czyjego pliku pochodzi wiadomość
    f.close();
}

void cleanup(int signum)
{
    unregister_user(name);
    append_text("[System] podany użytkownik wyszedł z konwersacji");
    string cmd = "rm " + my_file;
    system(cmd.c_str());
    exit(0);
}

void exit_cleanup()
{
    unregister_user(name);
}

void clear_input()
{
    // \033[A  -> przesuń kursor o jedną linię w górę
    // \033[2K -> wyczyść całą tę linię
    // \r      -> wróć na początek linii
    cout << "\033[A\033[2K\r" << flush;
}

string get_host_id() // te dwie funkcje są na odwrót alfabetycznie bo ta druga używa pierwszej
{
    struct stat info;

    if (stat(main_file.c_str(), &info) != 0) {
        perror("stat");
        return "error";
    }

    // 2. Zamieniamy UID na nazwę użytkownika
    struct passwd *pw = getpwuid(info.st_uid);
    if (pw != nullptr)
    {
        return string(pw->pw_name);
    } else
    {
        // Jeśli nie znaleziono nazwy, zwracamy UID jako tekst
        return to_string(info.st_uid);
    }
}

void create_connection_to_host() // tworzy plik
{
    // utworzenie pliku
    string cmd = "touch " + my_file;
    system(cmd.c_str());

    // odebranie praw czytania komukolwiek
    if (chmod(my_file.c_str(), 0600) == -1)
    {
        perror("Błąd chmod");
    }

    // nadanie praw czytania hostowi
    string host_id = get_host_id();
    cmd = "setfacl -m u:" + host_id + ":rw " + my_file;
    system(cmd.c_str());
}

void reader() // funkcja czytająca wiadomości z głównego pliku, za który odpowiada host
{
    // ?? czekaj aż plik się pojawi
    ifstream f;
    while (true)
    {
        f.open(main_file);
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
        return;
    }

    int wd = inotify_add_watch(fd, main_file.c_str(), IN_MODIFY);
    if (wd == -1)
    {
        perror("inotify_add_watch");
        close(fd);
        return;
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

int main(int argc, char* argv[]) // program potrzebuje nazwy użytkownika i nazwy chatu
{
    if (argc != 3)
    {
        perror("Błędna liczba argumentów!\n");
        return 1;
    }

    signal(SIGINT, cleanup);
    signal(SIGHUP, cleanup);

    atexit(exit_cleanup);

    name = argv[1];
    string arg2 = argv[2];
    main_file = "/tmp/chat_group-" + arg2;
    my_file = main_file + "-" + name;

    register_user(name); // jeżeli użytkownik jest już w lobby to nie stanie się nic

    // utworzenie swojego pliku i nadanie uprawnień
    create_connection_to_host();

    //ogarnięcie czytania głównego pliku
    thread t(reader);

    system("clear");
    cout << "[System] pomyślnie dołączono do czatu " << arg2 << endl;

    string line;
    while (getline(cin, line))
    {
        if (!is_str_empty(line))
            append_text(line);
            clear_input();
        cout << "> ";
    }

    t.join();
}