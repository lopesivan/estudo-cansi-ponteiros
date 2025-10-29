// g++ -std=c++17 -O2 -Wall -Wextra -pedantic matrix_uptr_v2.cpp -o app
#include <cstdlib>    // malloc, free
#include <cstdint>    // uint8_t
#include <iostream>
#include <memory>
#include <type_traits> // std::alignment_of (C++17: use std::alignment_of_v<T>)
#include <limits>

template <typename T>
using MallocUPtrRows = std::unique_ptr<T*, decltype (&std::free)>;

// helpers de overflow (simples)
static bool mul_overflow (size_t a, size_t b, size_t& out)
{
    if (a == 0 || b == 0)
    {
        out = 0;
        return false;
    }
    if (a > std::numeric_limits<size_t>::max() / b) return true;
    out = a * b;
    return false;
}
static bool add_overflow (size_t a, size_t b, size_t& out)
{
    if (a > std::numeric_limits<size_t>::max() - b) return true;
    out = a + b;
    return false;
}

template <typename T>
MallocUPtrRows<T> make_matrix (size_t rows, size_t cols)
{
    if (rows == 0 || cols == 0) return {nullptr, &std::free};

    // tamanho da tabela de ponteiros (rows * T*)
    size_t header_size;
    if (mul_overflow (rows, sizeof (T*), header_size))
        return {nullptr, &std::free};

    // tamanho da área de dados (rows * cols * T)
    size_t elems, data_size;
    if (mul_overflow (rows, cols, elems)) return {nullptr, &std::free};
    if (mul_overflow (elems, sizeof (T), data_size)) return {nullptr, &std::free};

    // padding para alinhar a área de dados a alignof(T)
    const size_t align = alignof (T);
    size_t padding = 0;
    if (align > 1)
    {
        size_t mis = header_size % align;
        if (mis) padding = align - mis;
    }

    // total = header + padding + dados
    size_t header_pad, total;
    if (add_overflow (header_size, padding, header_pad)) return {nullptr, &std::free};
    if (add_overflow (header_pad,  data_size, total))   return {nullptr, &std::free};

    auto* raw = static_cast<uint8_t*> (std::malloc (total));
    if (!raw) return {nullptr, &std::free};

    // constrói a unique_ptr que vai liberar com free
    MallocUPtrRows<T> row_table (reinterpret_cast<T**> (raw), &std::free);

    // dados contíguos começam após header+padding
    auto* data = reinterpret_cast<T*> (raw + header_pad);

    // ajusta as linhas
    for (size_t i = 0; i < rows; ++i)
        row_table.get()[i] = data + i * cols;

    return row_table; // RAII: free automático
}

int main()
{
    // Exemplo 3x2 de int (igual ao C)
    auto matriz = make_matrix<int> (3, 2);
    if (!matriz)
    {
        std::cerr << "Falha ao alocar matriz.\n";
        return EXIT_FAILURE;
    }

    // usar normalmente: mat[i][j]
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 2; ++j)
            matriz.get()[i][j] = static_cast<int> (i * 10 + j);

    for (size_t i = 0; i < 3; ++i)
    {
        for (size_t j = 0; j < 2; ++j)
            std::cout << matriz.get()[i][j] << ' ';
        std::cout << '\n';
    }

    // nada a fazer: free() é chamado automaticamente pelo deleter
}
