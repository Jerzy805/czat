#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h> // Potrzebne do chmod

#define MAX 256

char my_file[100];
char other_file[100];
char my_name[50];

// Wątek do czytania nowych wiadomości od kolegi
void *reader(void *arg)
{
    FILE *f;
    char line[MAX];

   //printf("Czekam na wiadomości w pliku: %s...\n", other_file);

    // Czekaj, aż plik kolegi zostanie utworzony
    while (1)
    {
        f = fopen(other_file, "r");
        if (f != NULL) break;
        usleep(500000); // czekaj 0.5s
    }

    // Najpierw wypisz to, co już jest w pliku
    while (fgets(line, MAX, f))
    {
        printf("%s", line);
    }

    // Tryb śledzenia pliku na żywo
    while (1)
    {
        if (fgets(line, MAX, f) != NULL)
        {
            printf("%s", line);
            fflush(stdout);
        }
        else
        {
            // Jeśli nie ma nowych linii, wyczyść błąd EOF i czekaj
            clearerr(f);
            usleep(200000);
        }
    }

    fclose(f);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Użycie: %s <twoja_nazwa> <nazwa_kolegi>\n", argv[0]);
        return 1;
    }

    strcpy(my_name, argv[1]);

    // Ścieżki do plików w /tmp
    snprintf(my_file, sizeof(my_file), "/tmp/msg_%s", argv[1]);
    snprintf(other_file, sizeof(other_file), "/tmp/msg_%s", argv[2]);

    // Tworzymy wątek czytający
    pthread_t tid;
    if (pthread_create(&tid, NULL, reader, NULL) != 0) {
        perror("Błąd tworzenia wątku");
        return 1;
    }

    char msg[MAX];
    FILE *f;

    //printf("Zalogowano jako %s. Możesz pisać wiadomości:\n", my_name);

    while (1)
    {
        if (fgets(msg, MAX, stdin) != NULL)
        {
            // Otwieramy plik w trybie dopisywania (append)
            f = fopen(my_file, "a");
            if (f != NULL)
            {
                // KLUCZOWE: Nadajemy uprawnienia 0666 (rw-rw-rw-)
                // Dzięki temu kolega będzie mógł czytać ten plik, 
                // nawet jeśli nie jest jego właścicielem.
                chmod(my_file, 0666);

                fprintf(f, "[%s] %s", my_name, msg);
                fclose(f);
            }
            else
            {
                perror("Błąd zapisu do pliku");
            }
        }
    }

    return 0;
}
