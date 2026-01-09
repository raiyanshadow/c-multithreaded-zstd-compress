/*
 * This program takes in no command-line arguments (if you put some in, they're
 * ignored).
 * It then reads in up to 30 characters from the keyboard (stopping early if you
 * press Enter), and prints that to standard output.
 *
 * There is an error in this file either in the code or in the comments. See
 * if you can figure out what is wrong. It is not a serious error that might 
 * potentially lead to incorrect output or the process aborting.
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
    char message[30];
    printf("Type in a message:\n");

    /* stdin should always be open, so any failure is a problem. */
    if(fgets(message, 30, stdin) == NULL) {
        perror("Could not read from stdin\n");
        exit(1);
    }

    printf("Your input: %s\n", message);
    return 0;
}
