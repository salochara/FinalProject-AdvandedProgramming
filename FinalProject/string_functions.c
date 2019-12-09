/*
   Functions for working with arrays of characters

    Gilberto Echeverria
    gilecheverria@yahoo.com
    01/02/2017
*/

#include "string_functions.h"

// Read a string using fgets, and remove the trailing '\n'
size_t inputString(char * string, int size, FILE * file_ptr)
{
    size_t length = 0;

    if ( fgets(string, size, file_ptr) != NULL )
    {
        length = strlen(string);
        // Change the last character in the string, if it is a '\n'
        if (length < size-1 && *(string + length - 1) == '\n')
        {
            // Set it as a null character
            *(string + length - 1) = '\0';
            // Reduce the length of the string
            length--;
        }
    }
    else
    {
        perror("Unable to read string from standard input\n");
        exit(EXIT_FAILURE);
    }

    return length;
}

// Get rid of the newline in the input buffer
void clearBufferEnter()
{
    char garbage;

    // Discard the newline left over in the buffer
    // From: http://c-faq.com/stdio/stdinflush2.html
    while((garbage = getchar()) != '\n' && garbage != EOF)
        /* discard */ ;
}

