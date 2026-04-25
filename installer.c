#include <stdlib.h>

int main()
{
    system("g++ chat.cpp send_file.cpp get_file.cpp -o chat -pthread");
    system("g++ call.cpp -o call");
    system("g++ grouphost.cpp  -o grouphost -pthread");
    system("g++ groupjoin.cpp -pthread -o groupjoin");
    //system ("cp chat ..");
    //system ("cp call ..");
    //system ("cp grouphost ..");
    //system ("cp groupjoin ..");
}