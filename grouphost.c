#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdbool.h>

char main_file[20];

void create_connection(int users, char user_ids[users][20])
{
    char filename[20];
    sprintf(filename, "/tmp/chat_group-%d", users+1);
    strcpy(main_file, filename)
    FILE *f = fopen(filename, "w");

    if (!f)
    {
        perror("fopen");
        return;
    }

    fclose(f);

    // 2. ustawienie chmod 600 (na wszelki wypadek)
    if (chmod(filename, 0600) == -1)
    {
        perror("chmod");
        return;
    }

    // 3. nadanie ACL użytkownikom (read)
    
    int i;

    for (i = 0; i < users; i++)
    {
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", user_ids, filename);

        if (system(cmd) == -1)
        {
            perror("system");
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc%2 != 0)
    {
        printf("Nieprawidłowa liczba argumentów!\n");
        return 1;
    }

    char my_name[20];
    strcpy(my_name, argv[1]);

    int users = (argc - 2) / 2;

    char nicks[users][20];
    char user_ids[users][20];

    // wczytywanie nicków użytkowników

    int i;

    // wczytywanie nicków
    for (int i = 0; i < users; i++)
    {
        strcpy(nicks[i], argv[2 + i]);
    }

    // wczytywanie id
    for (int i = 0; i < users; i++)
    {
        strcpy(user_ids[i], argv[2 + users + i]);
    }

    create_connection(users, user_ids);
    
    printf("supcio, %d\n", users);

    
}