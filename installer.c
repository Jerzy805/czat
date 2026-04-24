#include <stdio.h>
#include <stdlib.h>

int main()
{
    system("gcc chat.c send_file.c get_file.c -o chat");
    system("gcc -pthread grouphost.c -o grouphost");
    system("gcc -pthread groupjoin.c -o groupjoin");
    system("gcc call.c -o call");
    system("clear");
    printf("skompilowano wszystkie pliki (chyba)\n");
}
