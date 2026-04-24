#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <array>
#include <memory>

using namespace std;
namespace fs = filesystem;
using namespace fs;

string name;
string found_friends[20];
int count; // liczba znalezionych przyjaciół

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

string get_id(const string& filename)
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
    if(index >= count)
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
    cout << "Podaj swój nick:\n";
    cin >> name;

    count = get_existing();

    // tworzenie menu

    system("clear");
    int i = 0, option;

    if (count > 0)
        {
            for (i = 0; i < count; i++)
                cout << i + 1 << ". " << found_friends[i] << endl;
        }

        cout << i + 1 << ". Nowy czat\n";
        cout << i + 2 << ". Hostuj czat grupowy\n";
        cout << i + 3 << ". Dołącz do czatu grupowego\n";

        cin >> option;

        system("clear");

        if (option == i + 1) // nowy czat
        {
            string friend_name, id;
            cout << "Podaj nick rozmówcy:\n";
            cin >> friend_name;
            cout << "Podaj jego nazwę w spk:\n";
            cin >> id;

            create_connection(friend_name, id);

            execlp("./chat", "chat", name.c_str(), friend_name.c_str(), id.c_str(), NULL);
        }
        else if (option == 1 + 2) // hostowanie czatu grupowego
        {

        }
        else if (option == 1 + 3) // dołączanie do czatu grupowego
        {

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
                count = get_existing();
            }
            else
            {
                cout << "Niepoprawna komenda\n";
                return 1;
            }
        }
}