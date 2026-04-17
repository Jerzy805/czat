#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> // do wątków
#include <dirent.h> // do ścieżek plików 
#include <fcntl.h> // do nadawania uprawnień plikom
#include <sys/stat.h> //dla chmod

void get_file(const char *filename)
{
    // wczytanie zawartości wysłanego pliku do zmiennej text

    char fullname[80];

    snprintf(fullname, sizeof(fullname), "/tmp/%s", filename); // -> faktyczna ścieżka wysłanego pliku

    FILE *source = fopen(fullname, "r");

    if (!source)
    {
        perror("fopen");
        return;
    }

    fseek(source, 0, SEEK_END);
    long size = ftell(source);

    if (size < 0)
    {
        perror("ftell");
        fclose(source);
        return;
    }


    rewind(source);

    char *text = malloc(size + 1);
    if (!text)
    {
        fclose(source);
        perror("malloc");
        return;
    }

    if (fread(text, 1, size, source) != size)
    {
        perror("fread");
        free(text);
        fclose(source);
        return;
    }

    text[size] = '\0';
    fclose(source);

    // umieszczenie zawartości pliku w nowym pliku, tym razem w folderze głównym

    FILE *dest = fopen(filename, "w");

    if (!dest)
    {
        perror("fopen");
        free(text);
        return;
    }

    if (fwrite(text, 1, size, dest) != size)
    {
        perror("fwrite");
    }

    fclose(dest);

    free(text);
}