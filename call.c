#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <locale.h>
#include <stdbool.h>
#include <pwd.h>

char name[50], friend_name[50], id[20]; // id -> chodzi o id drugiego rozmówcy lub hosta czatu grupowego
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

void get_id(const char *filename)
{
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "getfacl %s", filename);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen");
        return;
    }

    char line[100];

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "user:", 5) == 0 && strstr(line, "::") == NULL) {
            sscanf(line, "user:%19[^:]", id);
            break;
        }
    }

    pclose(fp);
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
    
    system(cmd);
    
    sprintf(cmd, "rm /tmp/'chat_%s-%s!full'", name, person);
    
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
    
    
    int counter = show_existing();
    
    system("clear");
    
    // wypisywanie menu konwersacji
    printf("Wybierz opcję:\n");
        
        int i = 0;
        
        if (counter > 0)
        {
            for (i = 0; i < counter; i++)
            {
                printf("%d. %s\n", i+1, found_friends[i]);
            }
        }
        
        printf("%d. Nowy czat\n", i+1);
        printf("%d. Hostuj czat grupowy\n", i+2);
        printf("%d. Dołącz do czatu grupowego\n", i+3);
        
        printf("Żeby usunąć daną konwersację, wybierz jej numer i wpisz ze znakiem -\n");
        printf("Żeby przesłać plik, w konwersacji użyj komendy: !==!, a następnie wprowadź nazwę pliku\n");
        
        int option;
        
        scanf("%d", &option);
        
        system("clear");
        
        if (option == (i+1)) // uruchomienie nowego czatu
        {   
            printf("Podaj nick drugiego rozmówcy: ");
            scanf("%s", friend_name);
            printf("Podaj jego nazwę w spk: ");
            scanf("%s", id); // id jest pobierane
            
            // utworzenie pliku i nadanie odpowiednich uprawnień
            
            char filename[60];
            
            sprintf(filename, "/tmp/chat_%s-%s", name, friend_name);
            
            create_connection(filename, id);
            
            execlp("./chat", "chat", name, friend_name, id, NULL);
        }
        else if (option == (i+2))
        {
            // obsługa hostowania czatu grupowego

            char user_ids[30][20];
            char nicks[30][20];
            char pauses[25];
            memset(pauses, '-', sizeof(pauses) - 1);
            pauses[sizeof(pauses) - 1] = '\0'; 

            int j, option, counter = 0;

            for (j = 0; j < 30; j++)
            {
                system("clear");
                printf("Wybierz opcję:\n");
                printf("%s\n", pauses);
                printf("1. Dodaj nowego użytkownika\n");
                printf("2. Zakończ dodawanie użytkowników\n");

                scanf("%d", &option);

                if (option == 1)
                {
                    char nick[20], id[20];
                    printf("Podaj nick użytkownika:\n");
                    scanf("%19s", nick);
                    printf("Podaj nazwę w spk użytkownika:\n");
                    scanf("%19s", id);

                    strcpy(user_ids[counter], id);
                    strcpy(nicks[counter], nick);

                    counter++;
                }
                else
                {
                    break;
                }

                if (counter == 0)
                {
                    perror("Nie dodałeś żadnych użytkowników\n");
                    return 1;
                }
            }
            
            // parsowanie nicków i ids

            char final_nicks[600]= "";
            char final_ids[600] = "";

            for (j = 0; j < counter; j++)
            {
                 sprintf(final_nicks + strlen(final_nicks), "%s ", nicks[j]);
                 sprintf(final_ids + strlen(final_ids), "%s ", user_ids[j]);
            }

            //execlp("./grouphost", "grouphost", name, nicks, user_ids, NULL);

            char command[1250];

            sprintf(command, "./grouphost %s %s %s", name, final_nicks, final_ids);

            system(command);
            return 0;
        }   
        else if (option == (i+3))
        {
            // obsługa dołączenia do czatu grupowego
            
            bool is_correct = false; // czy nazwa czatu jest poprawna
            char full_chat_name[50];
            
            do
            {
                char chat_name[30];
                printf("Podaj nazwę czatu:\n");
                scanf("%s", &chat_name);
                
                // sprawdzenie czy istnieje taki czat
                
                sprintf(full_chat_name, "/tmp/chat_group-%s", chat_name);
                
                if (access(full_chat_name, F_OK) != 0)
                {
                    printf("Nie istnieje czat o tej nazwie, spróbuj ponownie\n");
                }
                else
                    is_correct = true;
                
            } while (!is_correct);
            
            system("clear");

            struct stat st;
            
            if (stat(full_chat_name, &st) == 0) {
                struct passwd *pw = getpwuid(st.st_uid);
            
                if (pw != NULL) {
                    snprintf(id, sizeof(id), "%s", pw->pw_name);
                } else {
                    printf("getpwuid failed\n");
                }
            } else {
                perror("stat");
            }
            
            execlp("./groupjoin", "groupjoin", name, id, full_chat_name, NULL);

        }
        else if(option < 0 && option >= -1-i)
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
            
            char filename[60];

            snprintf(filename, sizeof(filename),
                     "/tmp/chat_%s-%s", name, found_friends[option - 1]);
            
            get_id(filename);
            
            system("clear");
            
            execlp("./chat", "chat", name, found_friends[option - 1], id, NULL);

        }
    
    return 0;
}
