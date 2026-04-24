#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdbool.h>
#define MAX 256
#define GROUP_FILE_BUF 128
#define MEMBER_FILE_BUF 160

char main_file[GROUP_FILE_BUF];
const char* send_file_signal = "!==!";
const char* add_new_user_signal = "!=";

void print_pauses(int times)
{
    for (int i = 0; i < times; i++)
        printf("-");
    printf("\n");
}

void *reader(void *arg) // jako argument przekazywana nazwa pliku, do którego pisze dany gość
{
    char *filename = (char *)arg;
    
    FILE *f;
    char line[MAX];

    // Czekaj na plik gościa
    while ((f = fopen(filename, "r")) == NULL) {
        usleep(500000);
    }
    free(filename);//xDDDD ALE niech bedzie ten while 

    // Przeskocz do końca, żeby nie czytać historii przy wejściu
    fseek(f, 0, SEEK_END);

    while (1) {
        if (fgets(line, MAX, f) != NULL) {
            // tu była obsługa wysyłania pliku
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
            // utrwalenie wiadomości innych gości
            FILE *temp = fopen(main_file, "a");
            if (temp != NULL) {//tu sie wywalalo jak zle wpisze
                fputs(line, temp);
                fclose(temp);
            }
        } else {
        
            clearerr(f);
            fflush(f);
            usleep(100000);
        }
    }
    return NULL;
}

void create_connection(int users, char user_ids[users][20], char chat_name[20]) // utworzenie pliku głównego czatu i nadanie uprawnień
{
    char filename[GROUP_FILE_BUF];
    snprintf(filename, sizeof(filename), "/tmp/chat_group-%s", chat_name);
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
            snprintf(filename, sizeof(filename), "/tmp/chat_group-%s", chat_name);
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

    // 2. ustawienie chmod 600 (na wszelki wypadek), czm nie 0644?
    if (chmod(filename, 0600) == -1)
    {
        perror("chmod");
        return;
    }

    // 3. nadanie ACL użytkownikom (read)
    
    int i;

    for (i = 0; i < users; i++)
    {
        char cmd[200];
        snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", user_ids[i], filename);

        if (system(cmd) == -1)
        {
            perror("system");
            return;
        }
    }
}

void add_connection(char user[20], char name[20])
{
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", user, main_file);

    if (system(cmd) == -1)
    {
        perror("system");
        return;
    }
    
    pthread_t tid;
    char *filename = malloc(MEMBER_FILE_BUF);
    if (filename == NULL)
    {
        perror("malloc");
        return;
    }

    snprintf(filename, MEMBER_FILE_BUF, "%s-%s", main_file, name);

    if (pthread_create(&tid, NULL, reader, filename) != 0)
    {
        perror("pthread_create");
        free(filename);
        return;
    }

    pthread_detach(tid);
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
    

    // skopiowane z chat.c, kod do wysyłania wiadomości
    
    pthread_t threads[users];
    
    for (int i = 0; i < users; i++)
    {
        char *filename = malloc(MEMBER_FILE_BUF);
        if (filename == NULL)
        {
            perror("malloc");
            return 1;
        }

        snprintf(filename, MEMBER_FILE_BUF, "%s-%s", main_file, nicks[i]);

        if (pthread_create(&threads[i], NULL, reader, filename) != 0)
        {
            perror("pthread_create");
            free(filename);
            return 1;
        }
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    char msg[MAX];
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(msg, MAX, stdin) != NULL) {
            // Usuwamy znak nowej linii z końca msg, jeśli jest
            msg[strcspn(msg, "\n")] = 0;

            FILE *f = fopen(main_file, "a");
            if (f != NULL)
            {
                if (strcmp(msg, add_new_user_signal) == 0) // obsługa dodawania nowych ludzi
                {
                    char new_id[20], name[20];
                    printf("Podaj nazwę w spk użytkownika:\n");
                    scanf("%19s", new_id);
                    printf("Podaj nick użytkownika:\n");
                    scanf("%19s", name);

                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);//bo tu musi musi poczekac na cokolwiek
                    
                    add_connection(new_id, name);
                    
                    sprintf(msg, "[System] dodano nowego użytkownika %s", new_id);
                }
                
                // Zapisujemy do pliku (dodajemy \n, bo usunęliśmy go wyżej)
                fprintf(f, "[%s] %s\n", my_name, msg);
                fclose(f);
                
                // Do historii lokalnej też z nową linią
                char hist_msg[MAX+10];
                sprintf(hist_msg, "%s\n", msg);
                //update_history(hist_msg); // nie wiem czy w czasie grupowym takie coś ma w ogóle sens
            }
        }
    }
}
