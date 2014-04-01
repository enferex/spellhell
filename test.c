#include <stdio.h>

char *good = "I am spelled well.";
char *bad = "I am noooot spelled well.";

int main(void)
{
    char  literal1[] = "This is misssspelled literal";
    char *literal2   = "This is another misssspelled literal";
    char *literal3   = "This is spelled perfectly!";
    printf("This is a crapppily spelled string literal\n");
    printf("%s\n", "hello");
    return 0;
}
