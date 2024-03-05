#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_WORD_SIZE 100 // Maximum length of a word
#define PATH_MAX 100 // Maximum length of a word



int main (int argc, char *argv[]) {

    if (argc < 3) {
        perror("error");
        return 1;
    }

    int dictFile = open(argv[1], O_RDONLY);

    if (dictFile < 0) {
        perror("Error opening file");
        return 1;
    }

    for (int i = 2; i <= argc; i++) {

        struct stat pathStat;
        int r = stat(argv[i], &pathStat);

        if (r < 0) { 
            perror("Error checking file/directory");
            return 1;

        }
        if (S_ISREG(pathStat.st_mode)) {
        // regular file

        process_file(argv[i]);
        
        } else if (S_ISDIR(pathStat.st_mode)) {
        // directory
        } else {

        }

    }



    close(dictFile);


}

    void process_file(const char *filename) {
        int file = open(filename, O_RDONLY);
        if (file < 0) {
            perror("Error opening file");
            return;
        }

        char word[MAX_WORD_SIZE]; // Buffer to store the word

        // Read words from the file until EOF is reached
        while (scanf("%99s", word) == 1) {
            // Process the word (e.g., check spelling)
        }

        close(file);
    }




// 1. Finding and opening all the specified files, including directory traversal

// 2. Reading the file and generating a sequence of position-annotated words
// 3. Checking whether a word is contained in the dictionary