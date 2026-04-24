#include <unistd.h>

int main()
{
    system("g++ chat.cpp send_file.cpp get_file.cpp -o chat -pthread");
    system("g++ call.cpp -o call");
}