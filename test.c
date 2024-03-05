#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  // Include errno.h for error handling
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_WORD_SIZE 100 // Maximum length of a word
#define PATH_MAX 100 // Maximum length of a word
#define MAX_NUM_OF_WORDS 134335 //maximum number of dictionary words

int binarySearch(char *buffer, char *word, off_t size);

int main(int argc, char *argv[]) {
    if (argc != 2) {
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

    char *buffer = (char *) malloc((int)fileSize); // Double the buffer size to ensure enough space
    int total_bytes_read = 0;
    int bytes_read;

    while ((bytes_read = read(dictFile, buffer + total_bytes_read, (int)fileSize)) > 0) {
        // Check if buffer is full or if the last character read is a newline
    }
    char *bruh = "gut's";
    int searchResult = binarySearch(buffer, bruh, fileSize);

    free(buffer);

    // Other code...

    close(dictFile);
    return 0;
}


    int binarySearch(char *buffer, char *word, off_t size) {
        int low = 0;
        int high = (int)size - 1;
        int mid = (low + high) / 2;

        while (low <= high) {
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
        return -1;

    }
