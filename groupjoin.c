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

char my_name[20];
char host_id[20];
char full_chat_file_name[GROUP_FILE_BUF];
char my_file[MEMBER_FILE_BUF];

void *reader(void *arg)
{
    (void)arg;

    FILE *f;
    char line[MAX];

    while ((f = fopen(full_chat_file_name, "r")) == NULL) {
        usleep(500000);
    }

    // Przeskocz do końca, żeby nie czytać historii przy wejściu
    fseek(f, 0, SEEK_END);

    while (1) {
        if (fgets(line, MAX, f) != NULL)
        {
            // tu była obsługa wysyłania pliku
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
        } else {
            clearerr(f);
            usleep(100000);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // nie ma opcji żeby była zła liczba argumentów, jako że są przekazywane przez inny program
    strcpy(my_name, argv[1]);
    strcpy(host_id, argv[2]);
    snprintf(full_chat_file_name, sizeof(full_chat_file_name), "%s", argv[3]);

    snprintf(my_file, sizeof(my_file), "%s-%s", full_chat_file_name, my_name);

    // utworzenie pliku i nadanie uprawnień do czytania hostowi

    FILE *f = fopen(my_file, "w");

    if (!f)
    {
        perror("fopen");
        return 1;
    }
    fclose(f);

    // odebranie jakichkolwiek praw wszystkim innym
    if (chmod(my_file, 0600) == -1)
    {
        perror("chmod");
        return 1;
    }

    // nadanie uprawnień do czytania hostowi
    char cmd[200];
    snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", host_id, my_file);

    if (system(cmd) == -1)
    {
        perror("system");
        return 1;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(msg, MAX, stdin) != NULL) {
            // Usuwamy znak nowej linii z końca msg, jeśli jest
            msg[strcspn(msg, "\n")] = 0;

            FILE *f = fopen(my_file, "a");
            if (f != NULL)
            {
                // tu była obsługa wysyłania pliku    
                // Zapisujemy do pliku (dodajemy \n, bo usunęliśmy go wyżej)
                fprintf(f, "[%s] %s\n", my_name, msg);
                fclose(f);
                
                // Do historii lokalnej też z nową linią
                char hist_msg[MAX+10];
                sprintf(hist_msg, "%s\n", msg);
                //update_history(hist_msg);
            }
        }
    }
}
