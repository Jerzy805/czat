#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>

using namespace std;

void send_file(string filename, string id)
{
    string newname = "/tmp/" + filename;
    string cmd = "cp " + filename + " /tmp/";
    system(cmd.c_str());

    // ogarnięcie uprawnień

    if (chmod(newname.c_str(), 0600) == -1)
    {
        perror("chmod");
    }

    cmd = "setfacl -m u:" + id + ":r " + newname;
    
    if (system(cmd.c_str()) == -1)
    {
        perror("system");
    }
}