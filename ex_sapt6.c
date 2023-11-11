#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void print_permissions(mode_t mode) {
    char permissions[10] = "---------";
    if (mode & S_IRUSR) permissions[0] = 'R';
    if (mode & S_IWUSR) permissions[1] = 'W';
    if (mode & S_IXUSR) permissions[2] = 'X';
    if (mode & S_IRGRP) permissions[3] = 'R';
    if (mode & S_IWGRP) permissions[4] = 'W';
    if (mode & S_IXGRP) permissions[5] = 'X';
    if (mode & S_IROTH) permissions[6] = 'R';
    if (mode & S_IWOTH) permissions[7] = 'W';
    if (mode & S_IXOTH) permissions[8] = 'X';

    printf("%s", permissions);
}

int main(int argc, char *argv[]) {
    // Verifică numărul corect de argumente
    if (argc != 2) {
        write(STDERR_FILENO, "Usage: ./program <fisier_intrare>\n", 35);
        exit(EXIT_FAILURE);
    }

    // Deschide fișierul BMP pentru citire
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
        exit(EXIT_FAILURE);
    }

    // Citeste header-ul BMP
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
    if (bytesRead == -1) {
        perror("Eroare la citirea header-ului");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Extragerea informațiilor din header
    unsigned int *width = (unsigned int *)(buffer + 18);
    unsigned int *height = (unsigned int *)(buffer + 22);
    off_t fileSize = lseek(fd, 0, SEEK_END);

    // Obține informații despre fisier folosind stat
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Obține timpul ultimei modificări sub forma de string
    char timeString[20];
    struct tm *tm_info = localtime(&fileStat.st_mtime);
    strftime(timeString, sizeof(timeString), "%d.%m.%Y", tm_info);

    // Creează și deschide fișierul de statistici
    int statFile = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (statFile == -1) {
        perror("Eroare la deschiderea fisierului de statistici");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Scrie informațiile în fișierul de statistici
    dprintf(statFile, "nume fisier: %s\n", argv[1]);
    dprintf(statFile, "inaltime: %u\n", *height);
    dprintf(statFile, "lungime: %u\n", *width);
    dprintf(statFile, "dimensiune: %ld\n", (long)fileSize);
    dprintf(statFile, "identificatorul utilizatorului: %d\n", fileStat.st_uid);
    dprintf(statFile, "timpul ultimei modificari: %s\n", timeString);
    dprintf(statFile, "contorul de legaturi: %ld\n", (long)fileStat.st_nlink);

    // Drepturile de acces
    dprintf(statFile, "drepturi de acces user: ");
    print_permissions(fileStat.st_mode & S_IRWXU);
    dprintf(statFile, "\ndrepturi de acces grup: ");
    print_permissions(fileStat.st_mode & S_IRWXG);
    dprintf(statFile, "\ndrepturi de acces altii: ");
    print_permissions(fileStat.st_mode & S_IRWXO);
    dprintf(statFile, "\n");

    // Închide fișierele deschise
    close(fd);
    close(statFile);

    return 0;
}
