#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(2, "Usage: ./program <fisier_intrare>\n", 34);
        exit(-1);
    }

    char *fisier_intrare = argv[1];

    int fd_i = open(fisier_intrare, O_RDONLY);
    if (fd_i == -1)
    {
        perror("Eroare la deschiderea fisierului de intrare\n");
        return 1;
    }

    const char *ext = strrchr(fisier_intrare, '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        perror("Fisierul nu este bmp\n");
        exit(-1);
    }

    int width, height;
    // Seek la pozitia unde se afla informatiile de inaltime si latime
    lseek(fd_i, 18, SEEK_SET);

    if (read(fd_i, &width, sizeof(int)) != sizeof(int))
    {
        perror("Eroare la citirea latimii\n");
        close(fd_i);
        return 1;
    }

    if (read(fd_i, &height, sizeof(int)) != sizeof(int))
    {
        perror("Eroare la citirea inaltimii\n");
        close(fd_i);
        return 1;
    }

    char *fileName = strrchr(fisier_intrare, '/');
    if (fileName == NULL)
    {
        fileName = (char *)fisier_intrare;
    }
    else
    {
        fileName++;
    }

    struct stat file_info;
    if (fstat(fd_i, &file_info) == -1)
    {
        perror("Eroare la obtinerea info");
        close(fd_i);
        return 1;
    }

    int outputFile = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outputFile == -1)
    {
        perror("Eroare la deschiderea fisierului de intrare\n");
        close(fd_i);
        return 1;
    }

    char stats[1024];
    int numChars = snprintf(stats, sizeof(stats),
                            "Nume fisier: %s\nInaltime: %d\nLatime: %d\nDimensiune: %lu\nIdentificatorul Utilizatorului: %d\nTimpul ultimei modificari: %lu\nContorul de legaturi: %lu\n",
                            fileName, height, width, (unsigned long)file_info.st_size, file_info.st_uid, (unsigned long)file_info.st_mtime, (unsigned long)file_info.st_nlink);

    numChars += snprintf(stats + numChars, sizeof(stats) - numChars,
                         "Drepturi de acces user: %c %c %c\n",
                         (file_info.st_mode & S_IRUSR) ? 'R' : '-',
                         (file_info.st_mode & S_IWUSR) ? 'W' : '-',
                         (file_info.st_mode & S_IXUSR) ? 'X' : '-');

    numChars += snprintf(stats + numChars, sizeof(stats) - numChars,
                         "Drepturi de acces grup: %c %c %c\n",
                         (file_info.st_mode & S_IRGRP) ? 'R' : '-',
                         (file_info.st_mode & S_IWGRP) ? 'W' : '-',
                         (file_info.st_mode & S_IXGRP) ? 'X' : '-');

    numChars += snprintf(stats + numChars, sizeof(stats) - numChars,
                         "Drepturi de acces altii: %c %c %c\n",
                         (file_info.st_mode & S_IROTH) ? 'R' : '-',
                         (file_info.st_mode & S_IWOTH) ? 'W' : '-',
                         (file_info.st_mode & S_IXOTH) ? 'X' : '-');

    write(outputFile, stats, numChars);
    close(fd_i);
    close(outputFile);

    return 0;
}
