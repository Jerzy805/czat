#include <string>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

void get_file(string filename)
{
    string fullname = "/tmp/" + filename;

    ifstream source(fullname);

    if (!source)
    {
        perror("ifstream");
        return;
    }

    string text((istreambuf_iterator<char>(source)), istreambuf_iterator<char>());
    source.close();

    ofstream dest(filename);

    dest << text;
    dest.close();

    if (filename.find("tar.gz") != string::npos) // obsługa odbierania folderu
    {
        string cmd = "tar xzf " + filename;
        system(cmd.c_str());
        cmd = "rm " + filename;
        system(cmd.c_str());
    }
}