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

void convert_to_grayscale(const char *input_path, const char *output_path)
{
    FILE *input_file = fopen(input_path, "rb");
    if (!input_file)
    {
        perror("Error opening input file");
        exit(-1);
    }

    FILE *output_file = fopen(output_path, "wb");
    if (!output_file)
    {
        perror("Error opening output file");
        fclose(input_file);
        exit(-1);
    }

    BMPHeader bmpHeader;
    fread(&bmpHeader, sizeof(BMPHeader), 1, input_file);
    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output_file);

    Pixel *pixels = malloc(bmpHeader.width * bmpHeader.height * sizeof(Pixel));
    if (!pixels)
    {
        perror("Error allocating memory for pixels");
        fclose(input_file);
        fclose(output_file);
        exit(-1);
    }

    fread(pixels, sizeof(Pixel), bmpHeader.width * bmpHeader.height, input_file);

    for (int i = 0; i < bmpHeader.width * bmpHeader.height; i++)
    {
        uint8_t gray = (uint8_t)(0.299 * pixels[i].red + 0.587 * pixels[i].green + 0.114 * pixels[i].blue);
        pixels[i].red = gray;
        pixels[i].green = gray;
        pixels[i].blue = gray;
    }

    fwrite(pixels, sizeof(Pixel), bmpHeader.width * bmpHeader.height, output_file);

    free(pixels);
    fclose(input_file);
    fclose(output_file);
}

void process_bmp(const char *filename, const BMPHeader *bmpHeader, const char *output_dir)
{
    printf("Processing BMP file: %s\n", filename);
    printf("Width: %d\n", bmpHeader->width);
    printf("Height: %d\n", bmpHeader->height);
    printf("File Size: %u\n", bmpHeader->fileSize);

    char output_path_grayscale[BUFFER_SIZE];
    snprintf(output_path_grayscale, BUFFER_SIZE, "%s/%s_grayscale.bmp", output_dir, filename);

    convert_to_grayscale(filename, output_path_grayscale);
}

void process_file(const char *input_path, const char *output_dir, char c)
{
    const char *filename_with_path = basename((char *)input_path);
    char *filename = strdup(filename_with_path);
    if (filename == NULL)
    {
        perror("Error allocating memory for filename");
        exit(-1);
    }

    char *fileExtensionPointer = strrchr(filename, '.');
    if (fileExtensionPointer != NULL)
    {
        *fileExtensionPointer = '\0';
    }

    int fd = open(input_path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        free(filename);
        exit(-1);
    }

    BMPHeader bmpHeader;
    ssize_t bytesRead = read(fd, &bmpHeader, sizeof(BMPHeader));
    if (bytesRead == -1)
    {
        perror("Error reading BMP header");
        close(fd);
        free(filename);
        exit(-1);
    }

    if (bmpHeader.signature != 0x4D42)
    {
        printf("Not a valid BMP file: %s\n", filename);
        close(fd);
        free(filename);
        exit(-1);
    }

    process_bmp(filename, &bmpHeader, output_dir);

    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1)
    {
        perror("Error getting file information");
        close(fd);
        free(filename);
        exit(-1);
    }

    char timeString[20];
    struct tm *tm_info = localtime(&fileStat.st_mtime);
    strftime(timeString, sizeof(timeString), "%d.%m.%Y", tm_info);

    char stat_filename[BUFFER_SIZE];
    snprintf(stat_filename, BUFFER_SIZE, "%s/%s_statistica.txt", output_dir, filename);
    int statFile = open(stat_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (statFile == -1)
    {
        perror("Error opening statistics file");
        close(fd);
        free(filename);
        exit(-1);
    }

    dprintf(statFile, "nume fisier: %s\n", filename);
    dprintf(statFile, "inaltime: %d\n", bmpHeader.height);
    dprintf(statFile, "lungime: %d\n", bmpHeader.width);
    dprintf(statFile, "dimensiune: %u\n", bmpHeader.fileSize);
    dprintf(statFile, "identificatorul utilizatorului: %d\n", fileStat.st_uid);
    dprintf(statFile, "timpul ultimei modificari: %s\n", timeString);
    dprintf(statFile, "contorul de legaturi: %ld\n", (long)fileStat.st_nlink);

    dprintf(statFile, "drepturi de acces user: ");
    print_permissions(fileStat.st_mode & S_IRWXU);
    dprintf(statFile, "\ndrepturi de acces grup: ");
    print_permissions(fileStat.st_mode & S_IRWXG);
    dprintf(statFile, "\ndrepturi de acces altii: ");
    print_permissions(fileStat.st_mode & S_IRWXO);
    dprintf(statFile, "\n");

    close(fd);
    close(statFile);

    // New code to create and manage child processes for content processing
    pid_t child_content;

    if (strstr(filename_with_path, ".bmp") == NULL)
    {
        child_content = fork();
        if (child_content == -1)
        {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }

        if (child_content == 0)
        {
            // Child process for content processing
            process_file(input_path, output_dir, c);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    free(filename);
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
