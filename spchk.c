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

int binarySearch(char *dictBuffer, char *wordBuffer, off_t dictBufferSize, int caseSensitive);

int spellCheckFile(const char *path, char* dictBuffer, off_t dictFileSize);

int spellCheckDirectory(const char *dirPath, char *dictBuffer, off_t dictFileSize);

int checkWord(char *word, char *dictBuffer, off_t dictFileSize, int wordLength);

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
        int result = 0;

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

// Using parameter 1 is same as it used to be
int binarySearch(char *dictBuffer, char *wordBuffer, off_t dictBufferSize, int caseSensitive) {
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
        mid = low + (high-low) / 2;

        // Find start of word
        while (mid > 0 && dictBuffer[mid-1] != '\n') mid--;

        // Find end of word
        int end = mid;
        while (end < dictBufferSize && dictBuffer[end] != '\n') end++;

        int cmpResult = caseSensitive ? strncmp(dictBuffer + mid, wordBuffer, wordSize) : strncasecmp(dictBuffer + mid, wordBuffer, wordSize); 
        
        if (cmpResult == 0) {
            if (end - mid != wordSize || (end - mid == wordSize && dictBuffer[mid + wordSize] != '\n')) {
                cmpResult = dictBuffer[mid + wordSize] - '\n'; 
            } 
        }

        if (cmpResult == 0) {
            // Word found
            return mid; // Return start index
        } else if (cmpResult < 0) {
            low = end + 1;
        } else {
            high = mid - 1;
        }            
    }
    return -1;
}

int checkWord(char *word, char *dictBuffer, off_t dictFileSize, int wordLength) {

    printf("Checking word: %s\n", word);
    if (binarySearch(dictBuffer, word, dictFileSize, 1) >= 0) {
        return 1; // 1. Check for exact match first
    }

    // If not try insensitive
    int index = binarySearch(dictBuffer, word, dictFileSize, 1);

    printf("Index: %d\n", index);

    if (index >= 0) {
        int dictWordLength = 0;
        while (dictBuffer[index + dictWordLength] != '\n' && dictBuffer[index + dictWordLength] != '\0') {
            dictWordLength++;
        }

        // Get the matched word from the dictionary
        char *dictWord = malloc(dictWordLength+1);
        strncpy(dictWord, dictBuffer + index, dictWordLength);
        dictWord[dictWordLength] = '\0';

        // If exact match return
        
        if (strcmp(dictWord, word) == 0) {
            // printf("Exact match verified for word '%s'\n", word);
            free(dictWord);
            return 1;
        } else {
            // printf("Exact match verification failed for word '%s' against dictionary word '%s'\n", word, dictWord);
        }

        // Check for capitalization
        int hasUpper = 0;
        for (int i = 0; i < dictWordLength; i++) {
            if (isupper(dictWord[i])) {
                hasUpper = 1;
                break;
            }
        }

        printf("Uppercase : %d\n", hasUpper);

        if (!hasUpper) {
             // Convert to all lowercase
            char *lowerCaseWord = strdup(word);

            for (int i = 0; i < wordLength; i++) {
                lowerCaseWord[i] = tolower(lowerCaseWord[i]);
            }

            // Check if word is all uppercase
            int allUpper = 1;
            for (int i = 0; i < wordLength; i++) {
                if (!isupper(word[i])) {
                    allUpper = 0;
                    break;
                }
            }

            // Check if word is capitalized
            int isCapital;

            if (isupper(word[0])) {
                isCapital = 1;
                for (int i = 1; i < wordLength; i++) {
                    if (isupper(word[i])) {
                        isCapital = 0;
                        break;
                    }
                }
            } else {
                isCapital = 0;
            }

            // Valid if all upper and lowercase is in dictionary
            if ((allUpper && binarySearch(dictBuffer, lowerCaseWord, dictFileSize, 1) >= 0)) {
                free(lowerCaseWord);
                free(dictWord);
                return 1; // Matching lowercase version is found
            }

            // Valid if not all uppercase but only first letter is
            if (!allUpper) {
                if (isCapital && binarySearch(dictBuffer, lowerCaseWord, dictFileSize, 1) >= 0) {
                    free(lowerCaseWord);
                    free(dictWord);
                    return 1;
                }
            }

            free(lowerCaseWord);
        } 
        else {
            printf("Capital letters present in dictionary word. Verifying '%s' against '%s'.\n", word, dictWord);

            for (int i = 0; i < wordLength; i++) {
                if (!isupper(word[i])) {
                    free(dictWord);
                    return 0;
                }
            }

            return 1; // All uppercase
        }

        free(dictWord);
    }

    return 0; // No match found

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
       int lineNum = 1;
       int colNum = 1; 
       int lineNumAtStart = lineNum;
       int colNumAtStart = colNum;
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
                   if (wordLength == 0) {
                       lineNumAtStart = lineNum;
                       colNumAtStart = colNum - 1;
                   }
                   if (wordLength < wordBufferSize - 1) {
                       wordBuffer[wordLength] = ch;
                       wordLength++;
                   } else {
                       // Word buffer overflow, handle error or resize buffer as needed
                   }
               } else { //if ch is a whitespace character
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
                       while (wordLength > 0 && !isalpha(wordBuffer[wordLength - 1 - j])) {
                           wordLength--;
                       }
                       wordBuffer[wordLength] = '\0';
                           int foundWord = binarySearch(dictBuffer, wordBuffer, dictFileSize, 0);
                           if (foundWord < 0) {
                               // Convert first letter to uppercase and the rest to lowercase
                               char *capitalizedWord = strdup(wordBuffer);
                               if (capitalizedWord != NULL) {
                                   capitalizedWord[0] = toupper(capitalizedWord[0]); // Capitalize first letter
                                   for (int i = 1; i < strlen(capitalizedWord); i++) {
                                       capitalizedWord[i] = tolower(capitalizedWord[i]); // Lowercase the rest
                                   }
                                   foundWord = binarySearch(dictBuffer, capitalizedWord, dictFileSize, 0);
                                   free(capitalizedWord);
                               }
                           }
                           if (foundWord < 0) {
                               char *lowerCaseWord = strdup(wordBuffer);
                               for (size_t i = 0; i < strlen(wordBuffer); i++) {
                                   lowerCaseWord[i] = tolower(lowerCaseWord[i]);
                               }
                               foundWord = binarySearch(dictBuffer, lowerCaseWord, dictFileSize, 0);
                           }


                           // If still 0
                           if (foundWord < 0) {
                               printf("%s (%d,%d): %s\n", path, lineNumAtStart, colNumAtStart, wordBuffer);
                           }


                           wordLength = 0; // Reset word length for the next word
                   }
               }
           }
       }


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
                       while (wordLength > 0 && !isalpha(wordBuffer[wordLength - 1 - j])) {
                           wordLength--;
                       }
                       wordBuffer[wordLength] = '\0';
                           int foundWord = binarySearch(dictBuffer, wordBuffer, dictFileSize, 0);
                           if (foundWord < 0) {
                               // Convert first letter to uppercase and the rest to lowercase
                               char *capitalizedWord = strdup(wordBuffer);
                               if (capitalizedWord != NULL) {
                                   capitalizedWord[0] = toupper(capitalizedWord[0]); // Capitalize first letter
                                   for (int i = 1; i < strlen(capitalizedWord); i++) {
                                       capitalizedWord[i] = tolower(capitalizedWord[i]); // Lowercase the rest
                                   }
                                   foundWord = binarySearch(dictBuffer, capitalizedWord, dictFileSize, 0);
                                   free(capitalizedWord);
                               }
                           }
                           if (foundWord < 0) {
                               char *lowerCaseWord = strdup(wordBuffer);
                               for (size_t i = 0; i < strlen(wordBuffer); i++) {
                                   lowerCaseWord[i] = tolower(lowerCaseWord[i]);
                               }
                               foundWord = binarySearch(dictBuffer, lowerCaseWord, dictFileSize, 0);
                           }


                           // If still 0
                           if (foundWord < 0) {
                               printf("%s (%d,%d): %s\n", path, lineNumAtStart, colNumAtStart, wordBuffer);
                           }


                           wordLength = 0; // Reset word length for the next word
                  
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

    