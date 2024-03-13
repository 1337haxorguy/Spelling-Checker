#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  // Include errno.h for error handling
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>


// 1. Finding and opening all the specified files, including directory traversal
// 2. Reading the file and generating a sequence of position-annotated words
// 3. Checking whether a word is contained in the dictionary

int binarySearch(char *buffer, char *word, off_t size);

    int spellCheckFile(const char *filePath, size_t bufferSize, 
        char *wordBuffer, size_t wordBufferSize, char* dictBuffer, off_t fileSize);

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dictionary_file>\n", argv[0]);
        return 1;
    }

    int dictFile = open(argv[1], O_RDONLY);
    if (dictFile < 0) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return 1;
    }

    struct stat dictStat;
    if (stat(argv[1], &dictStat) != 0) {
        fprintf(stderr, "Error getting file information: %s\n", strerror(errno));
        return 1;
    }

    off_t fileSize = dictStat.st_size;

    char *buffer = (char *) malloc((int)fileSize); 
    int total_bytes_read = 0;
    int bytes_read;

    bytes_read = read(dictFile, buffer + total_bytes_read, (int)fileSize);
    char *bruh = "apple";
    int searchResult = binarySearch(buffer, bruh, fileSize);

    close(dictFile);


    for (int i = 2; i < argc; i++) {
        struct stat fileStat;
        if (stat(argv[i], &fileStat) == 0) {
            if (S_ISREG(fileStat.st_mode)) {
                printf("%s is a regular file.\n", argv[i]);
                ssize_t bufferSize = 100;
                char* wordBuffer = (char*)malloc(bufferSize);
                int result = spellCheckFile(argv[i], 1024, buffer, bufferSize, buffer, fileSize);

                // Here you can perform operations specific to regular files
            } else if (S_ISDIR(fileStat.st_mode)) {
                printf("%s is a directory.\n", argv[i]);
                // Here you can perform operations specific to directories
            } else {
                printf("%s is neither a regular file nor a directory.\n", argv[i]);
            }
        } else {
            fprintf(stderr, "Error getting file information for %s: %s\n", argv[i], strerror(errno));
        }
    }


    free(buffer);

    // Other code...

    return 0;
}


    int binarySearch(char *buffer, char *word, off_t size) {
        int low = 0;
        int high = (int)size - 1;
        int mid = (low + high) / 2;

        while (low < high) {
            mid = (low + high) / 2;
            char* midWord = buffer + mid;
            while (*(midWord-1) != '\n' && *(midWord-1) != '\0') {
                midWord--;
            }

            char* endWord = midWord;
            while (*endWord != '\n' && *endWord != '\0') {
                endWord++;
            }

            // Print or process the word
            printf("Word: ");
            for (char* p = midWord; p < endWord; ++p) {
                putchar(*p);
            }
            putchar('\n');

            if (strncmp(midWord, word, endWord - midWord) == 0) {
                // Word found
                printf("FOUND WORD \n");
                return 1;
            } else if (strncmp(midWord, word, endWord - midWord) < 0) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }            

        }
        printf("COULDNT FIND IT \n");
        return -1;

    }

    int spellCheckFile(const char *path, size_t bufferSize, char *wordBuffer, size_t wordBufferSize, char* dictBuffer, off_t fileSize) {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Error opening file: %s\n", strerror(errno));
            return 1;
        }
        char buffer[bufferSize]; // Buffer for reading data from file
        ssize_t bytes_read;
        size_t wordLength = 0;
        while ((bytes_read = read(fd, buffer, bufferSize)) > 0) {
            for (ssize_t i = 0; i < bytes_read; i++) {
                char ch = buffer[i];
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                    // If the character is a letter, add it to the word buffer
                    if (wordLength < wordBufferSize - 1) {
                        wordBuffer[wordLength++] = ch;
                    } else {
                        // Word buffer overflow, handle error or resize buffer as needed
                    }
                } else {
                    // If the character is not a letter (e.g., space, punctuation),
                    // process the word and reset the word buffer
                    if (wordLength > 0) {
                        // Null-terminate the word
                        wordBuffer[wordLength] = '\0';
                        // Handle the word (e.g., spell-check it)
                        // For now, just print it
                        printf("Word: %s\n", wordBuffer);
                        binarySearch(buffer, wordBuffer, fileSize);
                        wordLength = 0; // Reset word length for the next word
                    }
                }
            }
        }

        close(fd);
        return 0;



    }
