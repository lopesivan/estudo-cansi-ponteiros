#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   // uint8_t, uintptr_t
#include <stdalign.h> // alignof

static int mul_overflow_size (size_t a, size_t b, size_t* out)
{
    if (a == 0 || b == 0)
    {
        *out = 0;
        return 0;
    }
    if (a > SIZE_MAX / b) return 1;
    *out = a * b;
    return 0;
}

static int add_overflow_size (size_t a, size_t b, size_t* out)
{
    if (a > SIZE_MAX - b) return 1;
    *out = a + b;
    return 0;
}

float** new_matrix (size_t dim1, size_t dim2)
{
    if (dim1 == 0 || dim2 == 0)
    {
        // defina a política: aqui retornamos NULL
        return NULL;
    }

    // tamanho da “camada” de ponteiros (linha)
    size_t layer_1_size;
    if (mul_overflow_size (dim1, sizeof (float*), &layer_1_size)) return NULL;

    // tamanho da área de dados (float)
    size_t data_elems;
    if (mul_overflow_size (dim1, dim2, &data_elems)) return NULL;

    size_t data_size;
    if (mul_overflow_size (data_elems, sizeof (float), &data_size)) return NULL;

    // padding para alinhar a área de dados a alignof(float)
    size_t padding = 0;
    {
        size_t misalign = layer_1_size % alignof (float);
        if (misalign) padding = alignof (float) - misalign;
    }

    // total = ponteiros + padding + dados
    size_t header_plus_pad;
    if (add_overflow_size (layer_1_size, padding, &header_plus_pad)) return NULL;

    size_t total_size;
    if (add_overflow_size (header_plus_pad, data_size, &total_size)) return NULL;

    uint8_t* raw = (uint8_t*) malloc (total_size);
    if (!raw) return NULL;

    float** rows = (float**) (void*) raw; // tabela de ponteiros (dim1 * float*)
    float*  data = (float*)  (void*) (raw + header_plus_pad);

    // ajusta os ponteiros de cada linha
    for (size_t i = 0; i < dim1; ++i)
    {
        rows[i] = data + i * dim2;
    }

    return rows;
}

int main (void)
{
    size_t m = 3, n = 2;
    float** mat = new_matrix (m, n);
    if (!mat)
    {
        fprintf (stderr, "Falha ao alocar matriz.\n");
        return EXIT_FAILURE;
    }

    // usa normalmente
    for (size_t i = 0; i < m; ++i)
        for (size_t j = 0; j < n; ++j)
            mat[i][j] = (float) (i * 10 + (int)j);

    for (size_t i = 0; i < m; ++i)
    {
        for (size_t j = 0; j < n; ++j)
            printf ("%g ", mat[i][j]);
        puts ("");
    }

    // um único free — libera tabela de ponteiros + padding + dados
    free (mat);
    return 0;
}
