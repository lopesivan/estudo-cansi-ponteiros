#include <stdio.h>
#include <stdlib.h>

int main (void)
{
    puts ("Hello, World!");

    int* ptr = malloc (3 * sizeof *ptr);
    if (!ptr)
    {
        fprintf (stderr, "Memory allocation failed (malloc).\n");
        return EXIT_FAILURE;
    }

    size_t new_count = 6;

    // realloc seguro: usa temporário
    int* tmp = realloc (ptr, new_count * sizeof *ptr);
    if (!tmp)
    {
        fprintf (stderr, "Memory allocation failed (realloc).\n");
        // 'ptr' ainda é válido aqui; escolha: liberar e sair,
        // ou seguir usando o tamanho antigo. Aqui, vou liberar e sair:
        free (ptr);
        return EXIT_FAILURE;
    }
    ptr = tmp;  // a partir daqui, só use 'ptr'

    // ... use 'ptr' aqui ...

    free (ptr);
    return EXIT_SUCCESS;
}
