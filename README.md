Angelo Dela Fuente add139

Christopher Chiaramonte cmc701

We have implemented three methods other than main. 
1. Binary Search
    We search through an inputted buffer for the specified word, if we 
    land on the middle of the word we move the buffer to the start of the word
2. Spell Check File
    We iterate through the entire file, calling a binary search for each word in the file
    We ignore all non-alpha characters at the beginning and end of each word
3. Spell Check Directory

In our main function we put the entire dictionary file into a buffer, to ensure speedy searching

For testing we test a file with words within it using different spellings, capitalizations, and punctuation
We also put a directory within a directory to spell check to ensure that the recursive directory travel 
functions properly