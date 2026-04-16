#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <locale.h>


char name[50], friend_name[50], id[17];
char found_friends[20][50];

int show_existing()
{
    char prefix[100];
    int counter = 0; // Licznik, żeby wiedzieć czy co znalelimy
    
    sprintf(prefix, "chat_%s-", name);
    
    DIR *d = opendir("/tmp/");
    struct dirent *dir;
    
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strncmp(dir->d_name, prefix, strlen(prefix)) == 0 && strchr(dir->d_name, '!') == NULL)
            {
                int len = strlen(prefix);
                
                char *friend_ptr = dir->d_name + len;
                
                printf("Znaleziono czat: %s\n", friend_ptr);
                strcpy(found_friends[counter], friend_ptr);
                counter++;
            }
        }
        closedir(d); // Zawsze zamykaj katalog!

        if (counter == 0)
        {
            printf("Nie znaleziono żadnych konwersacji dla użytkownika %s w /tmp/\n", name);
        }
        
        return counter;
    }
    
    perror("Błąd otwierania /tmp/"); // Wypisze dokładny powód błędu systemu
    return 1;
}

void create_connection(const char *filename, const char *username)
{
    // 1. utworzenie pliku (lub otwarcie jeli istnieje)
    int fd = open(filename, O_CREAT | O_WRONLY, 0600);
    if (fd == -1)
    {
        perror("open");
        return;
    }
    close(fd);

    // 2. ustawienie chmod 600 (na wszelki wypadek)
    if (chmod(filename, 0600) == -1)
    {
        perror("chmod");
        return;
    }

    // 3. nadanie ACL dla użytkownika (read)
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "setfacl -m u:%s:r %s", username, filename);

    if (system(cmd) == -1)
    {
        perror("system");
        return;
    }
}

void delete_convo(int convo)
{
    // funkcja usuwająca odpowiednie pliki
    
    char *person = found_friends[convo];
    char cmd[100];
    
    sprintf(cmd, "rm /tmp/chat_%s-%s", name, person);
    
    int status = system(cmd);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        printf("Pomyślnie usunięto czat z: %s\n", person);
    }
    else
    {
        printf("Nie udało się usunąć pliku\n");
    }

}

int main()
{
    setlocale(LC_ALL, "");

    printf("Podaj nick: ");
    // Poprawiony scanf: bez & i z ograniczeniem szerokoci
    if (scanf("%49s", name) != 1) return 1;
    
    printf("Szukam konwersacji dla: %s...\n", name);
    
    int counter = show_existing();
    
    // wypisywanie menu konwersacji
    printf("Wybierz czat:\n");
        
        int i = 0;
        
        if (counter > 0)
        {
            for (i = 0; i < counter; i++)
            {
                printf("%d. %s\n", i+1, found_friends[i]);
            }
        }
        
        printf("%d. Nowy czat, i =%d\n", i+1, i);
        
        printf("Żeby usunąć daną konwersację, wybierz jej numer i wpisz ze znakiem -\n");
        
        int option;
        
        scanf("%d", &option);
        
        system("clear");
        
        if (option == (i+1))
        {
            // uruchomienie nowego czatu
            printf("Podaj nick drugiego rozmówcy: ");
            scanf("%s", friend_name);
            printf("Podaj jego nazwę w spk: ");
            scanf("%s", id);
            
            // utworzenie pliku i nadanie odpowiednich uprawnień
            
            char filename[60];
            
            sprintf(filename, "/tmp/chat_%s-%s", name, friend_name);
            
            create_connection(filename, id);
            
            execlp("./chat", "chat", name, friend_name, NULL);
        }
        else if(option < 0)
        {
            delete_convo(-option - 1);
        }
        else
        {
            if (option > i)
            {
                printf("Niepoprawna opcja!\n");
                return 0;
            }
        
            execlp("./chat", "chat", name, found_friends[option - 1], NULL);
        }
    
    return 0;
}
