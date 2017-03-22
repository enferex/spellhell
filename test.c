#include <stdio.h>

char *good       = "I am spelled well.";
char *notgood    = "I am blajdlkj.";
const char bad[] = "I am noooot spelled well."; // This will fail

int main(void)
{
    char  literal1[] = "This is misssspelled literal"; // This will fail
    char *literal2   = "This is another misssspelled literal"; // This will fail
    char *literal3   = "This is spelled perfectly!";
    printf("This is a crapppily spelled string literal\n"); // This will fail
    printf("%s\n", bad); // This will fail
    return 0;
}
