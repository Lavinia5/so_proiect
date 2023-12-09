#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include<time.h>
#include<libgen.h>
#define BUFFER_SIZE 1024

typedef struct
{
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

typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

void print_permissions(mode_t mode)
{
    char permissions[10] = "---------";
    if (mode & S_IRUSR)
        permissions[0] = 'R';
    if (mode & S_IWUSR)
        permissions[1] = 'W';
    if (mode & S_IXUSR)
        permissions[2] = 'X';
    if (mode & S_IRGRP)
        permissions[3] = 'R';
    if (mode & S_IWGRP)
        permissions[4] = 'W';
    if (mode & S_IXGRP)
        permissions[5] = 'X';
    if (mode & S_IROTH)
        permissions[6] = 'R';
    if (mode & S_IWOTH)
        permissions[7] = 'W';
    if (mode & S_IXOTH)
        permissions[8] = 'X';

    printf("%s", permissions);
}



void process_directory(const char *dirname, const char *output_dir, char c)
{
    DIR *dir = opendir(dirname);
    if (!dir)
    {
        perror("Error opening directory");
        exit(-1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char path[BUFFER_SIZE];
        snprintf(path, BUFFER_SIZE, "%s/%s", dirname, entry->d_name);
        process_file(path, output_dir, c);
        if (entry->d_type == DT_DIR)
            process_directory(path, output_dir, c);
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Call the main processing function for the specified directory
    process_directory(argv[1], argv[2], argv[3][0]);

    return 0;
}
