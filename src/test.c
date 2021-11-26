#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief This function is used to read the given file content
 * 
 * @param file_name The name of the file to read
 * @return char* The content of the file
 */
char* read_file(char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* buffer = (char*) malloc(sizeof(char) * (file_size + 1));
    if (buffer == NULL) {
        printf("Error allocating memory\n");
        exit(1);
    }

    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';

    fclose(file);

    return buffer;
}

/**
 * @brief This function is used to read the given file content
 * 
 * @param file_name The name of the file to read
 * @return u_int8_t* The content of the file
 */
u_int8_t* read_file_as_bytes(char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    u_int8_t* buffer = (u_int8_t*) malloc(sizeof(u_int8_t) * (file_size + 1));
    if (buffer == NULL) {
        printf("Error allocating memory\n");
        exit(1);
    }

    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';

    fclose(file);

    return buffer;
}

/**
 * @brief This is the main function of the program
 * 
 * @param argc The number of arguments. Should be 2
 * @param argv The arguments. Should be the name of the file to read
 * @return int 
 */
int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    u_int8_t* content = read_file_as_bytes(argv[1]);

    for (int i = 0; i < strlen((char *) content); i++) {
        printf("%o ", content[i]);
    }

    //print bytes as binary


    return 0;
}