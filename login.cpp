#include <cstdlib>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

int main()
{
    const char* user = getenv("USER");

	if (user == nullptr)
	{
        return 1;
    }

    fs::path folder = fs::read_symlink("/proc/self/exe").parent_path();

    string id = string(user);
    string cmd = "ssh -t " + id + "@ssh-spk.if.uj.edu.pl \"cd " + folder.string() + " && ./call; exec bash\"";
    system(cmd.c_str());
}