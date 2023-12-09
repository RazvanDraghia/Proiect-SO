#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

void convertToGrayscale(const char *bmpPath, const char *outputPath) {
    int bmpFile = open(bmpPath, O_RDONLY);
    int bmpFileOut = open(outputPath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    unsigned char header[54];
    int width, height, pixelDataOffset;

    if (bmpFile == -1 || bmpFileOut == -1) {
        perror("Error opening BMP file");
        exit(-1);
    }

    read(bmpFile, header, 54);
    memcpy(&width, &header[18], sizeof(width));
    memcpy(&height, &header[22], sizeof(height));
    memcpy(&pixelDataOffset, &header[10], sizeof(pixelDataOffset));
    write(bmpFileOut, header, 54);

    lseek(bmpFile, pixelDataOffset, SEEK_SET);
    lseek(bmpFileOut, pixelDataOffset, SEEK_SET);

    for (int i = 0; i < height * width; i++) {
        unsigned char pixel[3];
        read(bmpFile, pixel, sizeof(pixel));
        unsigned char gray = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
        pixel[0] = pixel[1] = pixel[2] = gray;
        write(bmpFileOut, pixel, sizeof(pixel));
    }

    close(bmpFile);
    close(bmpFileOut);
}


void processFile(const char *filePath);

void processSymbolicLink(const char *linkPath);

void processDirectory(const char *dirPath,const char *outputDir);

void processFile(const char *filePath)
{
    int fd_i = open(filePath, O_RDONLY);
    if (fd_i == -1)
    {
        perror("Eroare la deschiderea fisierului de intrare\n");
        return;
    }

    const char *ext = strrchr(filePath, '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        perror("Fisierul nu este bmp\n");
        close(fd_i);
        return;
    }
     struct stat target_info;
    if (stat(filePath, &target_info) == -1)
    {
        perror("Eroare la obtinerea informatiilor despre fisier\n");
        return;
    }

    int width, height;
    lseek(fd_i, 18, SEEK_SET);

    if (read(fd_i, &width, sizeof(int)) != sizeof(int))
    {
        perror("Eroare la citirea latimii\n");
        close(fd_i);
        return;
    }

    if (read(fd_i, &height, sizeof(int)) != sizeof(int))
    {
        perror("Eroare la citirea inaltimii\n");
        close(fd_i);
        return;
    }

    char *fileName = strrchr(filePath, '/');
    if (fileName == NULL)
    {
        fileName = (char *)filePath;
    }
    else
    {
        fileName++;
    }

    int outputFile = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (outputFile == -1)
    {
        perror("Eroare la deschiderea fisierului de iesire\n");
        close(fd_i);
        return;
    }
    char mod_time[20];
    strcpy(mod_time, (ctime(&target_info.st_mtime)));

    char buffer[512];
    int len = sprintf(buffer, "Nume fisier: %s\nInaltime: %d\nLatime: %d\nDimensiune: %lu\nIdentificatorul Utilizatorului: %d\nTimpul ultimei modificari: %s\nControlul de legaturi: %lu\n", fileName, height, width,(unsigned long)target_info.st_size, target_info.st_uid, mod_time, (unsigned long)target_info.st_nlink);
    len += sprintf(buffer + len, "Drepturi de acces user: %c %c %c\n", (target_info.st_mode & S_IRUSR) ? 'R' : '-', (target_info.st_mode & S_IWUSR) ? 'W' : '-', (target_info.st_mode & S_IXUSR) ? 'X' : '-');
    len += sprintf(buffer + len, "Drepturi de acces grup: %c %c %c\n", (target_info.st_mode & S_IRGRP) ? 'R' : '-', (target_info.st_mode & S_IWGRP) ? 'W' : '-', (target_info.st_mode & S_IXGRP) ? 'X' : '-');
    len += sprintf(buffer + len, "Drepturi de acces altii: %c %c %c\n", (target_info.st_mode & S_IROTH) ? 'R' : '-', (target_info.st_mode & S_IWOTH) ? 'W' : '-', (target_info.st_mode & S_IXOTH) ? 'X' : '-');

    if (write(outputFile, buffer, len) != len)
    {
        perror("Eroare la scrierea in fisier\n");
    }

    close(fd_i);
    close(outputFile);
}

void processSymbolicLink(const char *linkPath)
{
    struct stat link_info;
    if (lstat(linkPath, &link_info) == -1)
    {
        perror("Eroare la obtinerea informatiilor despre legatura simbolica\n");
        return;
    }

    char *linkName = strrchr(linkPath, '/');
    if (linkName == NULL)
    {
        linkName = (char *)linkPath;
    }
    else
    {
        linkName++;
    }

    char targetPath[PATH_MAX];
    ssize_t targetSize = readlink(linkPath, targetPath, sizeof(targetPath) - 1);
    if (targetSize == -1)
    {
        perror("Eroare la citirea legaturii simbolice\n");
        return;
    }
    targetPath[targetSize] = '\0';

    struct stat target_info;
    if (stat(targetPath, &target_info) == -1)
    {
        perror("Eroare la obtinerea informatiilor despre fisierul target\n");
        return;
    }

    int outputFile = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (outputFile == -1)
    {
        perror("Eroare la deschiderea fisierului de iesire\n");
        return;
    }

    char buffer[512];
    int len = sprintf(buffer, "Nume legatura: %s\nDimensiune: %lu\nDimensiune fisier: %lu\n", linkName, (unsigned long)link_info.st_size, (unsigned long)target_info.st_size);
    len += sprintf(buffer + len, "Drepturi de acces user: %c %c %c\n", (target_info.st_mode & S_IRUSR) ? 'R' : '-', (target_info.st_mode & S_IWUSR) ? 'W' : '-', (target_info.st_mode & S_IXUSR) ? 'X' : '-');
    len += sprintf(buffer + len, "Drepturi de acces grup: %c %c %c\n", (target_info.st_mode & S_IRGRP) ? 'R' : '-', (target_info.st_mode & S_IWGRP) ? 'W' : '-', (target_info.st_mode & S_IXGRP) ? 'X' : '-');
    len += sprintf(buffer + len, "Drepturi de acces altii: %c %c %c\n", (target_info.st_mode & S_IROTH) ? 'R' : '-', (target_info.st_mode & S_IWOTH) ? 'W' : '-', (target_info.st_mode & S_IXOTH) ? 'X' : '-');

    if (write(outputFile, buffer, len) != len)
    {
        perror("Eroare la scrierea in fisier\n");
    }

    close(outputFile);
}

void processDirectory(const char *dirPath, const char *outputDir)
{
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        perror("Eroare la deschiderea directorului\n");
        exit(-1);
    }

    int outputFile = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outputFile == -1)
    {
        perror("Eroare la deschiderea fisierului de iesire\n");
        closedir(dir);
        exit(-1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        struct stat file_info;
        if (lstat(filePath, &file_info) == -1)
        {
            perror("Eroare la obtinerea informatiilor despre fisier\n");
            continue;
        }
        int pid=fork();
        if(pid == -1)
        {
            perror("Eroare la procesul fork");
            exit(-1);
        }
        else if(pid == 0)
        {
            int total_lines = 0;
            char outputFileName[1024];
            snprintf(outputFileName,sizeof(outputFile),"%s/%s_statistica.txt",dirPath, entry->d_name);
            outputFile=open(outputFileName,O_WRONLY | O_CREAT | O_TRUNC,   S_IRUSR| S_IWUSR);

        }
        if (S_ISREG(file_info.st_mode))
        {
            const char *ext = strrchr(filePath, '.');
            if (ext != NULL && strcmp(ext, ".bmp") == 0)
            {
                processFile(filePath);
                int convertToGrayscale_pid=fork();
                if(convertToGrayscale_pid == -1 )
                {
                    perror("eroare la procesul de fork pt grayscale:");
                    exit(EXIT_FAILURE);
                }
                else if(convertToGrayscale_pid == 0)
                {
                    convertToGrayscale(filePath,filePath);
                    exit(EXIT_FAILURE);
                }
                else processFile(filePath);
            }
            else
            {
                char *fileName = strrchr(filePath, '/');
                if (fileName == NULL)
                {
                    fileName = (char *)filePath;
                }
                else
                {
                    fileName++;
                }
		char buffer[1024];
		char mod_time[20];
		strcpy(mod_time,(ctime(&file_info.st_mtime)));
                int len = sprintf(buffer, "Nume fisier: %s\n Dimensiune:%lu\n Identificatorul utilizatorului: %d\n Timpul ultimei modificari: %s\n", fileName,(unsigned long)file_info.st_size,file_info.st_uid,mod_time);
		
                len += sprintf(buffer + len, "Identificatorul Utilizatorului: %d\n", file_info.st_uid);
                len += sprintf(buffer + len, "Drepturi de acces user: %c %c %c\n", (file_info.st_mode & S_IRUSR) ? 'R' : '-', (file_info.st_mode & S_IWUSR) ? 'W' : '-', (file_info.st_mode & S_IXUSR) ? 'X' : '-');
                len += sprintf(buffer + len, "Drepturi de acces grup: %c %c %c\n", (file_info.st_mode & S_IRGRP) ? 'R' : '-', (file_info.st_mode & S_IWGRP) ? 'W' : '-', (file_info.st_mode & S_IXGRP) ? 'X' : '-');
                len += sprintf(buffer + len, "Drepturi de acces altii: %c %c %c\n", (file_info.st_mode & S_IROTH) ? 'R' : '-', (file_info.st_mode & S_IWOTH) ? 'W' : '-', (file_info.st_mode & S_IXOTH) ? 'X' : '-');

                if (write(outputFile, buffer, len) != len)
                {
                    perror("Eroare la scrierea in fisier\n");
                }
            }
        }
        else if (S_ISDIR(file_info.st_mode))
        {
            char *fileName = strrchr(filePath, '/');
            if (fileName == NULL)
            {
                fileName = (char *)entry->d_name;
            }
            else
            {
                fileName++;
            }
	    char buffer[512];

            int len = sprintf(buffer, "Nume director: %s\nIdentificatorul Utilizatorului: %d\n", fileName, file_info.st_uid);
            len += sprintf(buffer + len, "Drepturi de acces user: %c %c %c\n",
                           (file_info.st_mode & S_IRUSR) ? 'R' : '-',
                           (file_info.st_mode & S_IWUSR) ? 'W' : '-',
                           (file_info.st_mode & S_IXUSR) ? 'X' : '-');

            len += sprintf(buffer + len, "Drepturi de acces grup: %c %c %c\n",
                           (file_info.st_mode & S_IRGRP) ? 'R' : '-',
                           (file_info.st_mode & S_IWGRP) ? 'W' : '-',
                           (file_info.st_mode & S_IXGRP) ? 'X' : '-');

            len += sprintf(buffer + len, "Drepturi de acces altii: %c %c %c\n",
                           (file_info.st_mode & S_IROTH) ? 'R' : '-',
                           (file_info.st_mode & S_IWOTH) ? 'W' : '-',
                           (file_info.st_mode & S_IXOTH) ? 'X' : '-');

            if (write(outputFile, buffer, len) != len)
            {
                perror("Eroare la scrierea in fisier\n");
            }
        }
        else if (S_ISLNK(file_info.st_mode))
        {
            processSymbolicLink(filePath);
        }
    }

	

    close(outputFile);
    closedir(dir);
}

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        printf("eroare la argumente");
        exit(EXIT_FAILURE);
    }

    struct stat statbuf;
    if (stat(argv[1], &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        printf("Input directory error");
        exit(EXIT_FAILURE);
    }

    if (stat(argv[2], &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        printf("Output directory error");
        exit(EXIT_FAILURE);
    }

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(argv[1])) == NULL) {
        perror("opendir error");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        sprintf(path, "%s/%s", argv[1], entry->d_name);

        pid_t pid = fork();
        if (pid == 0) { // Proces copil
            if (strstr(entry->d_name, ".bmp")) {
                char outputPath[1024];
                sprintf(outputPath, "%s/%s_gray.bmp", argv[2], entry->d_name);
                convertToGrayscale(path, outputPath);
                exit(EXIT_SUCCESS);
            }
            // Scrie statistici pentru alte tipuri de fișiere
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0) { // Proces părinte
            int status;
            waitpid(pid, &status, 0);
            printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pid, WEXITSTATUS(status));
        }
        else {
            perror("fork error");
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);
    return 0;
}