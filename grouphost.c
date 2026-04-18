#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdbool.h>
#define MAX 256

char main_file[20];
const char* send_file_signal = "!==!";

void print_pauses(int times)
{
    for (int i = 0; i < times; i++)
        printf("-");
    printf("\n");
}

void reader(void *arg)
{
    char *filename = (char *)arg;
    
    FILE *f;
    char line[MAX];

    // Czekaj na plik kolegi (np. chat_emil-jerzy)
    while ((f = fopen(filename, "r")) == NULL) {
        //usleep(500000);
    }

    // Przeskocz do końca, żeby nie czytać historii przy wejściu
    fseek(f, 0, SEEK_END);

    while (1) {
        if (fgets(line, MAX, f) != NULL) {
            //writer(line);
            //if (check_msg(line))
//             {
//                 if (!file_handler())
//                 {
//                     printf("Nie przyjąłeś pliku\n");
//                     char text[50];
//                     sprintf(text, "[System] %s nie przyjął pliku", my_name);
//                     append_text(text);
//                 }
//                 else
//                 {
//                     printf("Pomyślnie przyjąłeś plik\n");
//                     char text[50];
//                     sprintf(text, "[System] %s pomyślnie przyjął plik", my_name);
//                     append_text(text);
//                 }
//             }
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
        } else {
            clearerr(f);
            usleep(100000);
        }
    }
    return NULL;
}

void create_connection(int users, char user_ids[users][20], char chat_name[20]) // utworzenie pliku głównego czatu i nadanie uprawnień
{
    char filename[20];
    sprintf(filename, "/tmp/chat_group-%s", chat_name);
    strcpy(main_file, filename);

    if (access(filename, F_OK) == 0)
    {
        printf("Istnieje już plik o tej nazwie, kontynuowanie z tą nazwą spowoduje usunięcie poprzedniej konwersacji grupowej.");
        print_pauses(25);
        printf("1. Zmień nazwę\n");
        printf("2. Kontynuuj\n");
        print_pauses(25);

        int option;
        scanf("%d", &option);

        if (option == 1)
        {
            printf("Nadaj nazwę czatu grupowego:\n");
            scanf("%19s", chat_name);
            sprintf(filename, "/tmp/chat_group-%s", chat_name);
            strcpy(main_file, filename);
        } 
    }

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

    char chat_name[20];
    printf("Nadaj nazwę czatu grupowego:\n");
    scanf("%19s", chat_name);

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

    create_connection(users, user_ids, chat_name);
    
    printf("supcio, %d\n", users);

    // skopiowane z chat.c, kod do wysyłania wiadomości
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    printf("supcio, %d\n", users);

    char msg[MAX];
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(msg, MAX, stdin) != NULL) {
            // Usuwamy znak nowej linii z końca msg, jeśli jest
            msg[strcspn(msg, "\n")] = 0;

            FILE *f = fopen(main_file, "a");
            if (f != NULL) {
                
                // Zapisujemy do pliku (dodajemy \n, bo usunęliśmy go wyżej)
                fprintf(f, "[%s] %s\n", my_name, msg);
                fclose(f);
                
                // Do historii lokalnej też z nową linią
                char hist_msg[MAX+10];
                sprintf(hist_msg, "%s\n", msg);
                //update_history(hist_msg); // nie wiem czy w czacie grupowym takie coś ma w ogóle sens
            }
        }
    }
}