#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <thread>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <sys/inotify.h>

using namespace std;

int users_count;
vector<string> nicks;
vector<string> ids;
string chat_name;
string main_file;

void append_text(string text, string user)
{
    string prefix = "[" + user + "] ";
    ofstream f(main_file, ios::app);
    if (!f.is_open())
    {
        perror("ofstream");
        return;
    }

    f << prefix << text << endl;
    f.close();
}

void reader(string user) 
{
    string filename = main_file + "-" + user;
    cout << "Reader: rozpoczęto czytanie: " << filename << endl;

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
            cout << line << endl;      // Wypisuje nową wiadomość
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
    ofstream f(main_file); // wielokrotne tworzenie jednego pliku, trzeba jakoś zmienić
    f.close();

    // odebranie wszelkich praw wszystkim
    if (chmod(main_file.c_str(), 0600) == -1)
    {
        perror("chmod");
        return 1;
    }

    string cmd = "setfacl -m u:" + id + ":r " + main_file;
    
    if (system(cmd.c_str()) == -1)
    {
        perror("system");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc % 2 != 0)
    {
        cout << "Błędna liczba argumentów!\n";
        return 1;
    }

    cout << "Podaj nazwę konwersacji groupowej:\n";
    cin >> chat_name; // przydało by się tu dodać obsługę żeby nazwa konwersacji nie była pusta lub biała

    main_file = "/tmp/chat_group-" + chat_name;

    users_count = (argc - 2) / 2;

    ids.resize(users_count);
    nicks.resize(users_count);

    // wczytanie id użytkowników
    for (int i = 0; i < users_count; i++)
    {
        ids[i] = argv[users_count + 2 + i];
        // nadanie praw czytania użytkownikom
        create_connection(ids[i]);
        cout << "nadano uprawnienia\n";
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

    // obsługa wysyłania wiadomości by się jeszcze przydała
}