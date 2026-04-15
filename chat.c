#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 256

char my_file[100];
char other_file[100];
char my_name[50];

// wątek do czytania nowych wiadomości
void *reader(void *arg)
{
    FILE *f;
    char line[MAX];

    // otwórz plik raz
    while ((f = fopen(other_file, "r")) == NULL)
    {
        sleep(1); // czekaj aż plik powstanie
    }

    // NAJPIERW pokaż istniejące wiadomości
    while (fgets(line, MAX, f))
    {
        printf("%s", line);
    }

    // potem przejdź w tryb "tail -f"
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

    snprintf(my_file, sizeof(my_file), "/tmp/msg_%s", argv[1]);
    snprintf(other_file, sizeof(other_file), "/tmp/msg_%s", argv[2]);

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    FILE *f;

    while (1)
    {
        if (fgets(msg, MAX, stdin) != NULL)
        {
            f = fopen(my_file, "a");
            if (f != NULL)
            {
                fprintf(f, "[%s] %s", my_name, msg);
                fclose(f);
            }
        }
    }

    return 0;
}
