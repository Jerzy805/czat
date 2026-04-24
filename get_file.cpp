#include <string>
#include <fstream>

using namespace std;

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
}