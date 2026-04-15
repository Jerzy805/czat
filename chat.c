#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#define MAX 256

char my_file[100];
char other_file[100];
char my_name[50];

// Wątek do czytania wiadomości od TEGO DRUGIEGO (tylko odczyt)
void *reader(void *arg)
{
    FILE *f;
    char line[MAX];

    // Czekaj na plik kolegi
    while ((f = fopen(other_file, "r")) == NULL)
    {
        usleep(500000); 
    }

    // Czytaj istniejące i nowe
    while (1)
    {
        if (fgets(line, MAX, f) != NULL)
        {
            printf("%s", line);
            fflush(stdout);
        }
        else
        {
            clearerr(f);
            usleep(200000);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Użycie: %s <twój_nick> <nick_kolegi>\n", argv[0]);
        return 1;
    }

    strcpy(my_name, argv[1]);

    // Definiujemy pliki:
    // MÓJ plik (do którego JA piszę)
    snprintf(my_file, sizeof(my_file), "/tmp/msg_%s", argv[1]);
    // PLIK KOLEGI (z którego JA tylko czytam)
    snprintf(other_file, sizeof(other_file), "/tmp/msg_%s", argv[2]);

    // Przed startem upewnijmy się, że mój plik ma uprawnienia do czytania dla kolegi
    FILE *init = fopen(my_file, "a");
    if (init) {
        fclose(init);
        chmod(my_file, 0644); // Właściciel pisze, inni tylko czytają
    }

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    while (1)
    {
        if (fgets(msg, MAX, stdin) != NULL)
        {
            // Otwieramy TYLKO SWÓJ plik (do którego mamy prawo własności)
            FILE *f = fopen(my_file, "a");
            if (f != NULL)
            {
                fprintf(f, "[%s] %s", my_name, msg);
                fclose(f);
            }
            else
            {
                perror("Błąd zapisu do Twojego pliku");
            }
        }
    }
    return 0;
}
