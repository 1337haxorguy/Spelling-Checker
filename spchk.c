#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>


// 1. Finding and opening all the specified files, including directory traversal
// 2. Reading the file and generating a sequence of position-annotated words
// 3. Checking whether a word is contained in the dictionary

typedef struct {
    char *original; // Original word
    char *modified; // Lowercased word
} DictionaryEntry;


DictionaryEntry* binarySearch(const char *word, DictionaryEntry *entries, size_t numDictEntries);

int spellCheckFile(const char *path, DictionaryEntry *dictionaryEntries, size_t numDictWords);

int spellCheckDirectory(const char *dirPath, DictionaryEntry *dictionaryEntries, size_t numDictWords);

int checkWord(char *word, DictionaryEntry *dictionaryEntires, size_t numDictWords);


// For binary search
int compareEntries(const void *a, const void *b) {
    const char *word = (const char *)a;
    const DictionaryEntry *entry = (const DictionaryEntry *)b;
    printf("Comparing: %s\n", entry->modified); // Corrected

    
    return  strcmp(word, entry->modified); // Compares lowercase version of words
}

int compareDictionaryEntry(const void *a, const void *b) {
    const DictionaryEntry *entryA = (const DictionaryEntry *)a;
    const DictionaryEntry *entryB = (const DictionaryEntry *)b;
    return strcmp(entryA->modified, entryB->modified);
}

// Makes string lowercase
char *toLower(const char *str) {
    char *lowerCaseStr = malloc(strlen(str) + 1);
    char *p = lowerCaseStr;
    while (*str != '\0') {
        *p++ = tolower((unsigned char)* str++);
    }
    *p = '\0';
    return lowerCaseStr; 
}
 
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

    // Count words in dict
    size_t numdDictWords = 0;
    for (char *p = dictBuffer; *p; p++) {
        if (*p == '\n') numdDictWords++;
    }

    // DictionaryEntry array
    DictionaryEntry *dictionaryEntries = malloc(sizeof(DictionaryEntry) * numdDictWords);
    if (!dictionaryEntries) {
        close(dictFile);
        free(dictBuffer);
        return EXIT_FAILURE;
    }

    // Populate dictionaryEntries
    char *line = strtok(dictBuffer, "\n");
    size_t index = 0;
    while (line != NULL && index < numdDictWords) {
        dictionaryEntries[index].original = strdup(line);
        dictionaryEntries[index].modified = toLower(line);
        line = strtok(NULL, "\n");
        index++;
    }

    // Sort dictionaryEntries
    qsort(dictionaryEntries, numdDictWords, sizeof(DictionaryEntry), compareDictionaryEntry);

    close(dictFile);

    // Loop through remaining args
    for (int i = 2; i < argc; i++) { 
        struct stat fileStat;
        int result = 0;

        if (stat(argv[i], &fileStat) != 0) { 
            fprintf(stderr, "Error getting file information for %s: %s\n", argv[i], strerror(errno));
            return EXIT_FAILURE;
        }

        if (S_ISREG(fileStat.st_mode)) { // File is regular
            printf("%s is a regular file.\n", argv[i]); // Has to be removed in final
            result = spellCheckFile(argv[i], dictionaryEntries, numdDictWords); // Spell check file with buffer array

        } else if (S_ISDIR(fileStat.st_mode)) { // File is a directory
            printf("%s is a directory.\n", argv[i]); // Has to be removed in final
            result = spellCheckDirectory(argv[i], dictionaryEntries, numdDictWords);
            
        } else {
            printf("%s is neither a regular file nor a directory.\n", argv[i]); // Has to be removed in final
        }

        if (result == -1) {
            return EXIT_FAILURE;
        }
    }

    // Free
    free(dictBuffer);
    for(size_t i = 0; i < numdDictWords; i++) {
        free(dictionaryEntries[i].original);
        free(dictionaryEntries[i].modified);
    }
    free(dictionaryEntries);

    return EXIT_SUCCESS;
}

// Using parameter 1 is same as it used to be
DictionaryEntry* binarySearch(const char *word, DictionaryEntry *entries, size_t numDictEntries) {
    int low = 0;
    int high = numDictEntries -1;


    while (low <= high) { 
        size_t mid = low + (high-low) / 2;
        int cmp = compareEntries(word, &entries[mid]);

        if(cmp == 0) {
            return &entries[mid]; // Word found
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }

    }

    return NULL;
}


int hasUpperCase(const char *str) {
    while (*str) {
        if (isupper((unsigned char)*str)) return 1;
        str ++;
    }

    return 0;
}

int isAllUpperCase(const char *str) {
    while (*str) {
        if (islower((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
 }

int checkWord(char *word, DictionaryEntry *dictionaryEntries, size_t numDictEntries) {
    char *wordLower = toLower(word);
    DictionaryEntry *foundEntry = binarySearch(wordLower, dictionaryEntries, numDictEntries);
    free(wordLower);

    if (foundEntry != NULL) {

        // 1. Check for exact match
        if (strcmp(foundEntry->original, word) == 0) {
            return 1;
        }

        // 2. Check if word from dictionary contains uppercase, if it does make sure word is all uppercase
        if (hasUpperCase(foundEntry->original)) { // If word in dictionary has uppercase
            if (isAllUpperCase(word)) { // Word must be all uppercase
                return 1;
            } else {
                return 0;
            }

        } else { // 3. No capitals in word
            if (isAllUpperCase(word)) {
                return 1; // We know the lowercase version is in dict
            }

            // Check if first letter is capital
            char *dictWordCapital = foundEntry->original;
            dictWordCapital[0] = toupper(dictWordCapital[0]);

            if (strcmp(word, dictWordCapital) == 0) {
                return 1;
            }
        }

    }

    return 0;

}
    


int spellCheckFile(const char *path, DictionaryEntry *dictionaryEntries, size_t numDictWords) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        close(fd);
        return EXIT_FAILURE;
    }


    size_t bufferSize = 1024;
    char *buffer = (char *)malloc(bufferSize);
    if (!buffer) {
        close(fd);
        return EXIT_FAILURE;
    }

    char wordBuffer[101];
    ssize_t bytesRead;
    int wordLength = 0;
    int lineNum = 1;
    int colNum = 0; 
    int startColNum = 1;
    int startLineNum = lineNum;

    while ((bytesRead = read(fd, buffer, bufferSize)) > 0) { // Read bytes from file into buffer
        for (ssize_t i = 0; i < bytesRead; i++) {
            char ch = buffer[i];

            if (ch == '\n') {
                lineNum++;  // Increment line number
                colNum = 0; // Reset column number when new line encountered
            } else {
                colNum++;
            }

            if (isalpha(ch) || (wordLength > 0 && (ch == '\'' || ch == '-'))) {
                if (wordLength == 0) { // Starting a new word
                    startColNum = colNum;
                    startLineNum = lineNum;
                }
                if (wordLength < 100) {
                    wordBuffer[wordLength++] = ch;
                }
            } else if (wordLength > 0) {
                wordBuffer[wordLength] = '\0'; // Null-terminate

                if (!checkWord(wordBuffer, dictionaryEntries, numDictWords)) {
                    printf("%s (%d,%d): %s\n", path, startLineNum, startColNum, wordBuffer);
                }

                wordLength = 0;
            }
        }
    }

        if (wordLength > 0) {
            wordBuffer[wordLength] = '\0'; // Null-terminate

            if (!checkWord(wordBuffer, dictionaryEntries, numDictWords)) {
                printf("%s (%d,%d): %s\n", path, lineNum, startColNum, wordBuffer);
            }

            wordLength = 0;
        }

    free(buffer);
    close(fd);

    return 0; // Successful operation

}


    int spellCheckDirectory(const char *dirPath, DictionaryEntry *dictionaryEntries, size_t numDictWords) {
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
                    spellCheckDirectory(path, dictionaryEntries, numDictWords); // Directory so call recursively
                } else if (S_ISREG(entryStat.st_mode)) {
                    // Regular file, make sure it ends with .txt
                    if (strstr(entry->d_name, ".txt") != NULL) { // Saw strstr online, pretty sure we can use??
                        // Call spell check
                        spellCheckFile(path, dictionaryEntries, numDictWords);
                    }
                }
            } else {
                return -1;
            }
        }

        closedir(dir);
        return 0;


    }

    