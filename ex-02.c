#include <stdio.h>
#include <stdlib.h>

int main (void)
{
    puts ("Hello, World!");
    char* o = "AAAABBBBCCCCDDDD";
    char* d = "MMMMMMMMMMMMMMMM";
    char* r = "RRRRRRRRRRRRRRRR";

    printf ("origem: %s\n",  o);
    printf ("destino: %s\n", d);
    scat (d, o);

    printf ("R: %s\n", r);
    printf ("origem: %s\n",  o);
    printf ("destino: %s\n", d);

    return EXIT_SUCCESS;
}
