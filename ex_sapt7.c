
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define BUFFER_SIZE 1024

typedef struct {
    uint16_t signature;     
    uint32_t fileSize;     
    uint32_t reserved;      
    uint32_t dataOffset;   
    uint32_t infoSize;     
    int32_t width;          
    int32_t height;         
    uint16_t planes;        
    uint16_t bitCount;       
    uint32_t compression;   
    uint32_t imageSize;      
    int32_t xPixelsPerM;    
    int32_t yPixelsPerM;     
    uint32_t colorsUsed;     
    uint32_t colorsImportant;
} BMPHeader;

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

void process_bmp(const char *filename, const BMPHeader *bmpHeader) {
    printf("Processing BMP file: %s\n", filename);
    printf("Width: %d\n", bmpHeader->width);
    printf("Height: %d\n", bmpHeader->height);
    printf("File Size: %u\n", bmpHeader->fileSize);
   
}

void process_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    BMPHeader bmpHeader;
    ssize_t bytesRead = read(fd, &bmpHeader, sizeof(BMPHeader));
    if (bytesRead == -1) {
        perror("Error reading BMP header");
        close(fd);
        return;
    }

    if (bmpHeader.signature != 0x4D42) {
        printf("Not a valid BMP file: %s\n", filename);
        close(fd);
        return;
    }

    process_bmp(filename, &bmpHeader);
    close(fd);
}

void process_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignora intrările "." și ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char path[BUFFER_SIZE];
        snprintf(path, BUFFER_SIZE, "%s/%s", dirname, entry->d_name);
        process_file(path);
        if (entry->d_type == DT_DIR)
            process_directory(path);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <director_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    process_directory(argv[1]);

    return 0;
}
