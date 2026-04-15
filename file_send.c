#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void send_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("fopen");
        return;
    }

    // Przesyłanie pliku
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        // Wysyłanie danych
    }

    fclose(f);
}