#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include "commands.h"
#include "file_send.h"

#define MAX 256
#define sciezka "/tmp/chat_%s-%s"
#define lolz printf("lolz\n");
//na testy
char my_file[150];
char other_file[150];
char my_name[50];

FILE *fileRead;
FILE *init;
FILE *fWrite;

void endAndClose(int signum){
    (void)signum;
    printf("Zamykam pliki...\n");
    unlink(my_file);
    unlink(other_file);
    if (fileRead) {
        fclose(fileRead);
        fileRead = NULL;
    }
    if (init) {
        fclose(init);
        init = NULL;
    }
    if (fWrite) {
        fclose(fWrite);
        fWrite = NULL;
    }
    exit(0);
}

void *reader() {

    char line[MAX];

    // Czekaj na plik kolegi (np. chat_emil-jerzy)
    while ((fileRead = fopen(other_file, "r")) == NULL) {
        usleep(500000);
    }

    // Przeskocz do końca, żeby nie czytać historii przy wejściu
    fseek(fileRead, 0, SEEK_END);

    while (1) {
        if (fgets(line, MAX, fileRead) != NULL) {
            printf("\r\033[K%s> ", line); // Wyświetl wiadomość i przywróć znak zachęty
            fflush(stdout);
        } else {
            clearerr(fileRead);
            usleep(100000);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    if (signal(2, endAndClose) == SIG_ERR) {
        perror("signal");
        return 1;
    }
    if (argc != 3) {
        printf("Użycie: %s <twój_nick> <nick_drugiego_użytkownika>\n", argv[0]);
        printf("Inne użycie: %s info\n", argv[0]);
        if (argc == 3)
        {   //to dosyc wazne bo wywala sie wszystko zrzut pamieci odwolanie poza obszar dostepny
            if(strcmp(argv[2], "info") == 0){
                printf("Informacje o czacie:\n");
                printf("Twój nick: %s\n", argv[1]);
                printf("Nick drugiego użytkownika: %s\n", argv[2]);
            }
            // tu trzeba dać dokładną instrukcję obsługi, jako że będą też czaty grupowe
            return 0;
        }
        printf("Jeżeli drugi użytkownik napisze coś w trakcie tego, jak coś pisałeś, wystarczy pisać dalej, tekst sam się uzupełni.\n");
        return 1;
    }

    strcpy(my_name, argv[1]);

    // Format: chat_JA-KOLEGA (to Twój plik do zapisu)
    snprintf(my_file, sizeof(my_file), sciezka, argv[1], argv[2]);
    // Format: chat_KOLEGA-JA (to plik kolegi, który czytasz)
    snprintf(other_file, sizeof(other_file), sciezka, argv[2], argv[1]);

    // Utwórz swój plik i daj innym prawo do czytania (0644)
    init = fopen(my_file, "a");
    if (init) {
        fclose(init);
        init = NULL;
        chmod(my_file, 0644);
    }

    printf("Rozpoczęto czat z %s\n", argv[2]);
    printf("Twój plik: %s\n", my_file);
    printf("Czytam z:  %s\n", other_file);

    // Przed startem upewnijmy się, że mój plik ma uprawnienia do czytania dla kolegi
    FILE *init = fopen(my_file, "a");
    if (init) {
        fclose(init);
        chmod(my_file, 0644); // Właściciel pisze, inni tylko czytają
    }

    pthread_t tid;
    pthread_create(&tid, NULL, reader, NULL);

    char msg[MAX];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(msg, MAX, stdin) != NULL) {
            fWrite = fopen(my_file, "a");
            if (fWrite != NULL) {
                fprintf(fWrite, "[%s] %s", my_name, msg);
                fclose(fWrite);
                fWrite = NULL;
            }
            else
            {
                perror("Błąd zapisu do Twojego pliku");
            }
        }
    }
    return 0;
}