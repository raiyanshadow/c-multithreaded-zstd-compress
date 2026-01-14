#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Return the number of repeated digits in number. Return -1
 * if numbers is NULL or length is less than 1. 
 *      numbers: an array of digits
 *      length: the number of digits in number
 * Your solution must have only one loop.
 * Note that main is written so that we can't test length < 1 or
 * numbers == NULL
 */

int repeated_digits(int *numbers, int length) {
    // TODO complete the function according to its description
    // remember to change the return value.
   return -9;
}

/* 
 * Modify numbers so that only digits that appear more than
 * once remain. The order of the digits in numbers when the
 * function returns will be the order in which duplicates 
 * were identified. Length is updated to reflect the new
 * length of numbers.
 * 
 * Your solution must have only one loop, and must modify 
 * numbers in place.
 * 
 * Examples:
 *  leave_repeated_digits([1, 2, 2, 1, 3, 2], 6) -> numbers = [2, 1], length = 2
 *  leave_repeated_digits([1, 2], 2) -> numbers = [], length = 0
 * 
 */
void leave_repeated_digits(int *numbers, int *length) {
    // TODO complete the function according to its description
}


// Do not change any code after this comment

/* Print the elements of numbers separated by a space.
 */
void print_array(int *numbers, int length) {
    for (int i = 0; i < length; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
}

/* This program takes one commandline argument, a string of digits
 * Since you are not expected to allowed to modify the main function,
 * you can assume your code will be tested with valid input.
 *
 * Sample usage:
 * $ gcc -Wall -g -o digits digits.c
 * $ ./digits 1313
 * 2
 * 1 3
 */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    /* Notes: the argument to the program is a string of digits, so
     * the starter code converts it to an array of integers before
     * passing it to the functions that you will write.

     * Note: argv[1][i] is a character, and we can do arithmetic on
     * characters.  For example, the ascii value of the character '3'
     * is 51 and the ascii value of '0' is 48, so '3' - '0' == 51 - 48 == 3
     */

    // Convert the argument to an array of digits, checking that the
    // characters are all digits.
    int len = strlen(argv[1]);
    int digits[len]; 

    for(int i = 0; i < len; i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            printf("Argument must be a number\n");
            return 1;
        } else {
            digits[i] = argv[1][i] - '0';
        }
    }

    // Print the number of repeated digits, followed by the array
    // of digits with only the repeated digits remaining
    printf("%d\n", repeated_digits(digits, len));
    leave_repeated_digits(digits, &len);
    print_array(digits, len);

    return 0;
}
