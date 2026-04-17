#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> // do wątków
#include <dirent.h> // do ścieżek plików 
#include <fcntl.h> // do nadawania uprawnień plikom
#include <sys/stat.h> //dla chmod

// char *send_file_signal = "!==!";

void send_file(const char *my_file, const char *filename, char id[30]) // pierwszy argument to plik, w którym zapisywane są własne wiadomości
{
    char cmd[100], newname[50];

    snprintf(cmd, sizeof(cmd), "cp %s /tmp/", filename);

    system(cmd); // -> przekopiowanie do tmp odpowiedniego pliku

    sprintf(newname, "/tmp/%s", filename);

    // zabranie wszystkim jakichkolwiek uprawnień

    if (chmod(newname, 0600) == -1)
    {
        perror("chmod");
        return NULL;
    }

    // nadanie odpowiednich uprawnień drugiemu rozmówcy

    snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", id, newname);

    system(cmd); // reszta dzieje się w pliku chat.c

    /* wysłanie wiadomości, która uruchomi u drugiego rozmówcy uruchomienie funkcji get_file();

    FILE *f = fopen(my_file);

    if (!f)
    {
        perror(fopen);
        return NULL;
    }

    fprintf(f, "");
    */
}