#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <thread>
#include <fcntl.h>

#define MAX_COUNT 20

using namespace std;

int users_count;
string nicks[MAX_COUNT];
string ids[MAX_COUNT];
string chat_name, main_file;

int create_connection(string id) // tworzy plik i nadaje uprawnienia
{
    ofstream f(main_file);
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

    // wczytanie nicków użytkowników
    for (int i = 0; i < users_count; i++)
    {
        nicks[i] = argv[2 + i];
    }

    // wczytanie id użytkowników
    for (int i = 0; i < users_count; i++)
    {
        ids[i] = argv[users_count + 2 + i];
    }

    // nadanie praw czytania użytkownikom
    for (int i = 0; i < users_count; i++)
    {
        create_connection(ids[i]);
    }
}