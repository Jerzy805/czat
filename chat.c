#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "send_file.h"
#include "get_file.h"

#define MAX 256

char my_file[150];
char other_file[150];
char full_file[150];
char my_name[50];
char friend_name[50];
char id[20];
char sent_file_name[50];
char *send_file_signal = "!==!";

bool check_msg(const char* msg)
{
    int len = strlen(msg);
    int chars_to_skip = 3 + strlen(friend_name);
    int i = chars_to_skip;

    if (i + 3 >= len)
        return false;

    if (msg[i] == '!' && msg[i+1] == '=' &&
        msg[i+2] == '=' && msg[i+3] == '!')
    {
        int j = 0;

        for (i = chars_to_skip + 4; i < len; i++)
        {
            sent_file_name[j++] = msg[i];
        }

        sent_file_name[j] = '\0';
        return true;
    }

    return false;
}

bool file_handler()
{
    printf("[System] %s chce ci wysłać plik %s, kontynuować [y/n]?", friend_name, sent_file_name);
    char option;
    scanf("%c", &option);

    if (option == 'y')
    {
        get_file(sent_file_name);
        return true;
    }
    return false;
}

void writer(char line[MAX]) // funkcja zapisująca lokalnie całą historię rozmowy
{
    FILE *f = fopen(full_file, "a");
    fprintf(f, "%s", line);
    fclose(f);
}

void show_last_history() {
    FILE *f = fopen(full_file, "r");
    if (f == NULL) return; // Jeśli plik nie istnieje (pierwsza rozmowa), nic nie rób

    char *lines[10];
    for (int i = 0; i < 10; i++) lines[i] = malloc(MAX);
    
    int count = 0;
    char temp[MAX];

    // Wczytujemy cały plik, trzymając w pamięci tylko 10 ostatnich linii
    while (fgets(temp, MAX, f) != NULL) {
        strcpy(lines[count % 10], temp);
        count++;
    }
    fclose(f);

    printf("--- 10 ostatnich wiadomości ---\n");
    
    // Obliczamy od którego momentu zacząć wypisywanie
    int start = (count > 10) ? (count % 10) : 0;
    int limit = (count > 10) ? 10 : count;

    for (int i = 0; i < limit; i++) {
        printf("%s", lines[(start + i) % 10]);
    }
    printf("-------------------------------\n");

    for (int i = 0; i < 10; i++) free(lines[i]);
}

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
            writer(line);
            if (check_msg(line))
            {
                if (!file_handler())
                {
                    printf("Nie przyjąłeś pliku\n");
                }
                else
                {
                    printf("Pomyślnie przyjąłeś plik\n");
                }
            }
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
        } else {
            clearerr(f);
            usleep(100000);
        }
    }
    return NULL;
}

void update_history(char msg[MAX]) // funkcja nadpisująca lokalne pliki po obu stronach
{
    FILE *f = fopen(full_file, "a");
    if (f != NULL)
    {
        fprintf(f, "[ty] %s", msg);
        fclose(f);
    }
}

int main(int argc, char *argv[]) {
    // if (argc != 4) {
    //     printf("Użycie: %s <twój_nick> <nick_drugiego_użytkownika>\n", argv[0]);
    //     printf("Inne użycie: %s info\n", argv[0]);
    //     printf("Jeżeli drugi użytkownik napisze coś w trakcie tego, jak coś pisałeś, wystarczy pisać dalej, tekst sam się uzupełni.\n");
    //     if (argc == 2 && strcmp(argv[1], "info") == 0)
    //     {
    //         // tu trzeba dać dokładną instrukcję obsługi, jako że będą też czaty grupowe
    //         return 0;
    //     }
        
    //     return 1;
    // }

    strcpy(id, argv[3]); // zapisanie w id loginu do spk rozmówcy

    strcpy(friend_name, argv[2]);

    strcpy(my_name, argv[1]);

    // Format: chat_JA-KOLEGA (to Twój plik do zapisu)
    snprintf(my_file, sizeof(my_file), "/tmp/chat_%s-%s", argv[1], argv[2]);
    // Format: chat_KOLEGA-JA (to plik kolegi, który czytasz)
    snprintf(other_file, sizeof(other_file), "/tmp/chat_%s-%s", argv[2], argv[1]);
    // Format: chat_JA-KOLEGA!full (pełna historia czatu)
    snprintf(full_file, sizeof(full_file), "/tmp/chat_%s-%s!full", argv[1], argv[2]);

    // Utwórz swój plik i daj innym prawo do czytania (0644)
    // FILE *init = fopen(my_file, "a");
    // if (init) {
    //     fclose(init);
    //     chmod(my_file, 0644);
    // }
    // zakomentowane, ponieważ tym się zajmuje plik call.c i daje prawo czytania jedynie drugiemu rozmócy
    
    show_last_history();

    printf("Rozpoczęto czat z %s\n", argv[2]);
    printf("Twój plik: %s\n", my_file);
    printf("Czytam z:  %s\n", other_file);

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    while (1) {
        //printf("> ");
        fflush(stdout);
        if (fgets(msg, MAX, stdin) != NULL) {
            FILE *f = fopen(my_file, "a");
            if (f != NULL) {
                if (strcmp(msg, send_file_signal)) // wychwycenie twojej chęci wysłania pliku
                {
                    char *file_to_send = malloc(50);
                    printf("Podaj nazwę pliku:\n");
                    scanf("%s", file_to_send);

                    send_file(my_file, file_to_send, id); // umiejscowienie pliku w odpowiednim miejscu

                    sprintf(msg, "!==![%s]", file_to_send); // zakomunikowanie rozmówcy, że chcę mu coś wysłać
                }
                fprintf(f, "[%s] %s", my_name, msg);
                fclose(f);
                update_history(msg);
            }
        }
    }
    return 0;
}
 // other_