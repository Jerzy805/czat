#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#define MAX 256

char my_file[150];
char other_file[150];
char my_name[50];

void *reader(void *arg) {
    FILE *f;
    char line[MAX];

    // Czekaj na plik kolegi (np. chat_emil-jerzy)
    while ((f = fopen(other_file, "r")) == NULL) {
        usleep(500000);
    }

    // Przeskocz do końca, żeby nie czytać historii przy wejściu
    fseek(f, 0, SEEK_END);

    while (1) {
        if (fgets(line, MAX, f) != NULL) {
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
        } else {
            clearerr(f);
            usleep(100000);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Użycie: %s <twój_nick> <nick_drugiego_użytkownika>\n", argv[0]);
        printf("Inne użycie: %s info\n", argv[0]);
        if (argc == 2 && argv[2] == "info")
        {
            // tu trzeba dać dokładną instrukcję obsługi, jako że będą też czaty grupowe
            return 0;
        }
        printf("Jeżeli drugi użytkownik napisze coś w trakcie tego, jak coś pisałeś, wystarczy pisać dalej, tekst sam się uzupełni.\n");
        return 1;
    }

    strcpy(my_name, argv[1]);

    // Format: chat_JA-KOLEGA (to Twój plik do zapisu)
    snprintf(my_file, sizeof(my_file), "/tmp/chat_%s-%s", argv[1], argv[2]);
    // Format: chat_KOLEGA-JA (to plik kolegi, który czytasz)
    snprintf(other_file, sizeof(other_file), "/tmp/chat_%s-%s", argv[2], argv[1]);

    // Utwórz swój plik i daj innym prawo do czytania (0644)
    FILE *init = fopen(my_file, "a");
    if (init) {
        fclose(init);
        chmod(my_file, 0644);
    }

    printf("Rozpoczęto czat z %s\n", argv[2]);
    printf("Twój plik: %s\n", my_file);
    printf("Czytam z:  %s\n", other_file);

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(msg, MAX, stdin) != NULL) {
            FILE *f = fopen(my_file, "a");
            if (f != NULL) {
                fprintf(f, "[%s] %s", my_name, msg);
                fclose(f);
            }
        }
    }
    return 0;
}
