#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  // Include errno.h for error handling
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>


// 1. Finding and opening all the specified files, including directory traversal
// 2. Reading the file and generating a sequence of position-annotated words
// 3. Checking whether a word is contained in the dictionary

int binarySearch(char *dictBuffer, char *wordBuffer, off_t dictBufferSize);

int spellCheckFile(const char *path, char* dictBuffer, off_t dictFileSize);

int spellCheckDirectory(const char *dirPath, char *dictBuffer, off_t dictFileSize);

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dictionary_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open first argument, path to dictionary
    int dictFile = open(argv[1], O_RDONLY);
    if (dictFile < 0) { // If doesn't open
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat dictStat; // Create stat struct to dictionary file
    if (stat(argv[1], &dictStat) != 0) {
        fprintf(stderr, "Error getting file information: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    off_t dictFileSize = dictStat.st_size; // Get dictionary file size

    char *dictBuffer = (char *) malloc((int)dictFileSize); // Allocate a buffer to hold amount of bytes as file
    int total_bytes_read = 0;
    int bytes_read;

    bytes_read = read(dictFile, dictBuffer + total_bytes_read, (int)dictFileSize); // Putting all data into buffer array
    
    if (bytes_read <= 0) {
        return EXIT_FAILURE;
    } 

    close(dictFile);

    // Loop through remaining args
    for (int i = 2; i < argc; i++) { 
        struct stat fileStat;
        int result;

        if (stat(argv[i], &fileStat) != 0) { 
            fprintf(stderr, "Error getting file information for %s: %s\n", argv[i], strerror(errno));
            return EXIT_FAILURE;
        }

        if (S_ISREG(fileStat.st_mode)) { // File is regular
            printf("%s is a regular file.\n", argv[i]); // Has to be removed in final
            result = spellCheckFile(argv[i], dictBuffer, dictFileSize); // Spell check file with buffer array

        } else if (S_ISDIR(fileStat.st_mode)) { // File is a directory
            printf("%s is a directory.\n", argv[i]); // Has to be removed in final
            result = spellCheckDirectory(argv[i], dictBuffer, dictFileSize);
            
        } else {
            printf("%s is neither a regular file nor a directory.\n", argv[i]); // Has to be removed in final
        }

        if (result == -1) {
            return EXIT_FAILURE;
        }
    }


    free(dictBuffer);

    // Other code...

    return EXIT_SUCCESS;
}


int binarySearch(char *dictBuffer, char *wordBuffer, off_t dictBufferSize) {
    int wordSize = 0;
    char* wordPtr = wordBuffer;
    while (*wordPtr != '\0') {
        wordPtr++;
        wordSize++;
    }

    int low = 0;
    int high = (int)dictBufferSize - 1;
    int mid;

    while (low <= high) { 
        mid = (low + high) / 2;
        char* midWord = dictBuffer + mid;
        while (*(midWord-1) != '\n' && *(midWord-1) != '\0') {
            midWord--;
        }

        char* endWord = midWord + wordSize;
        
        if (strncmp(midWord, wordBuffer, wordSize) == 0 && *(endWord) == '\n') {
            // Word found
            printf("word found: %s \n", wordBuffer);

            return 1;
        } else if (strncmp(midWord, wordBuffer, wordSize) < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }            

    }
    
    return -1;
}
    

    int spellCheckFile(const char *path, char* dictBuffer, off_t dictFileSize) {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            close(fd);
            return -1;
        } 

        size_t bufferSize = 1024;
        size_t wordBufferSize = 100;
        char* wordBuffer = (char*)malloc(wordBufferSize);

        char *buffer = (char *)malloc(bufferSize); // Buffer for reading data from file
        ssize_t bytes_read;
        size_t wordLength = 0;
        int lineNum = 0;
        int colNum = 1;  
        while ((bytes_read = read(fd, buffer, bufferSize)) > 0) { // Read bytes from file into buffer
            for (ssize_t i = 0; i < bytes_read; i++) {
                char ch = buffer[i];
                if (ch == '\n') {
                    lineNum++;  // Increment line number
                    colNum = 1; // Reset column number when new line encountered
                } else {
                    colNum++; // Increment column number
                }

                if ((!isspace(ch) && ch != '-')) {
                    // If the character is a letter, add it to the word buffer
                    if (wordLength < wordBufferSize - 1) {
                        wordBuffer[wordLength++] = ch;
                    } else {
                        // Word buffer overflow, handle error or resize buffer as needed
                    }
                } else {
                    if (wordLength > 0) {
                    int i = 0;
                    while (!isalpha(wordBuffer[i])) {
                        i++;
                    }
                    // Shift the word to the left to remove non-alphabetic characters
                    memmove(wordBuffer, wordBuffer + i, wordLength - i + 1);
                    wordLength -= i;

                    // Remove non-alphabetic characters from the end of the word
                    int j = 0;
                    while (wordLength > 0 && !isalpha(wordBuffer[wordLength - 1])) {
                        j++;
                        wordLength--;
                    }
                    wordBuffer[wordLength] = '\0';
                        // Handle the word (e.g., spell-check it)
                        // For now, just print it
                        int foundWord = binarySearch(dictBuffer, wordBuffer, dictFileSize);
                        if (foundWord < 0) {
                            printf("%s (%d,%d): %s\n", path, lineNum, colNum, wordBuffer);
                        }
                        wordLength = 0; // Reset word length for the next word
                    }
                }
            }
        }

        close(fd);
        free(wordBuffer);
        free(buffer);
        return 0;
    }

    int spellCheckDirectory(const char *dirPath, char *dictBuffer, off_t dictFileSize) {
        DIR *dir = opendir(dirPath);
        if (dir == NULL) {
            return -1;
        }


        struct dirent *entry;

        // Loop through entries in directory
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue; // Skip entries starting with .

            // Construct path for entry
            char path[1024];
            strcpy(path, dirPath);
            strcat(path, "/");
            strcat(path, entry->d_name);

            // Check if file or dir
            struct stat entryStat;

            if (stat(path, &entryStat) == 0) {
                if (S_ISDIR(entryStat.st_mode)) {
                    spellCheckDirectory(path, dictBuffer, dictFileSize); // Directory so call recursively
                } else if (S_ISREG(entryStat.st_mode)) {
                    // Regular file, make sure it ends with .txt
                    if (strstr(entry->d_name, ".txt") != NULL) { // Saw strstr online, pretty sure we can use??
                        // Call spell check
                        spellCheckFile(path, dictBuffer, dictFileSize);
                    }
                }
            } else {
                return -1;
            }
        }

        closedir(dir);
        return 0;


    }
