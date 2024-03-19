Angelo Dela Fuente add139
Christopher Chiaramonte cmc701

The Spelling Checker program is designed to help identify spelling errors in text files based off a provided dictionary.
We chose to implement a struct called DictionaryEntry to keep track of original dictionary words and their lowercase counterparts.
By doing this we were able to efficiently make sure all words in the dictionary including those with capitals were handeled correctly. 


Test Scenarios

Test 1: Single file check
We ran the program with a single test file with correct and incorrect words to ensure it detects the errors despite spaces, etc.

Test 2: Directory check
Created a directory with several directories and text files inside to ensure directory traversel was working properly.

Test 3: Case sensitivity
We specifically made files with words containing various cases like APPLE, apple, ApPle to make sure incorrect ways were identified.

Test 4: Large file check
We ran the program with a large file to make sure it finished in a reasonable amount of time.

Test 5: Special characters
Created text files with special characters and lots of spacing to make sure it was handled correctly.