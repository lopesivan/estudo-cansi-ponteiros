// g++ -std=c++20 -O2 -Wall -Wextra -pedantic matrix_uptr_v5.cpp -o app
#include <cstdlib>    // malloc, free
#include <cstdint>    // uint8_t
#include <iostream>
#include <memory>
#include <limits>
#include <span>
#include <cassert>

template <typename T>
using RowTableUPtr = std::unique_ptr<T*, decltype (&std::free)>;

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
RowTableUPtr<T> make_matrix (size_t rows, size_t cols)
{
    if (rows == 0 || cols == 0) return {nullptr, &std::free};

    size_t header_size;                       // rows * sizeof(T*)
    if (mul_overflow (rows, sizeof (T*), header_size)) return {nullptr, &std::free};

    size_t elems, data_size;                  // rows * cols * sizeof(T)
    if (mul_overflow (rows, cols, elems))      return {nullptr, &std::free};
    if (mul_overflow (elems, sizeof (T), data_size)) return {nullptr, &std::free};

    // padding para alinhar o bloco de dados a alignof(T)
    const size_t align = alignof (T);
    size_t mis = header_size % align;
    size_t padding = mis ? (align - mis) : 0;

    size_t header_pad, total;
    if (add_overflow (header_size, padding, header_pad)) return {nullptr, &std::free};
    if (add_overflow (header_pad, data_size, total))     return {nullptr, &std::free};

    auto* raw = static_cast<uint8_t*> (std::malloc (total));
    if (!raw) return {nullptr, &std::free};

    // tabela de linhas (T**) gerenciada por unique_ptr com deleter free
    RowTableUPtr<T> rows_table (reinterpret_cast<T**> (raw), &std::free);
    auto* data = reinterpret_cast<T*> (raw + header_pad);

    for (size_t i = 0; i < rows; ++i)
        rows_table.get()[i] = data + i * cols;

    return rows_table;
}

// -------- spans “achatados” (linearizam a matriz) --------
template <class T>
std::span<T> matriz_span (RowTableUPtr<T> const& rows, size_t r, size_t c)
{
    return rows ? std::span<T> (rows.get()[0], r * c) : std::span<T>();
}
template <class T>
std::span<const T> matriz_cspan (RowTableUPtr<T> const& rows, size_t r, size_t c)
{
    return rows ? std::span<const T> (rows.get()[0], r * c) : std::span<const T>();
}

// (opcional) helpers 2D <-> 1D
constexpr size_t ij_to_k (size_t i, size_t j, size_t cols)
{
    return i * cols + j;
}
constexpr std::pair<size_t,size_t> k_to_ij (size_t k, size_t cols)
{
    return {k / cols, k % cols};
}

int main()
{
    size_t R = 3, C = 2;
    auto mat = make_matrix<int> (R, C);
    if (!mat)
    {
        std::cerr << "Falha ao alocar matriz.\n";
        return EXIT_FAILURE;
    }

    // acesso 2D normal
    for (size_t i = 0; i < R; ++i)
        for (size_t j = 0; j < C; ++j)
            mat.get()[i][j] = int (10*i + j);

    for (size_t i = 0; i < R; ++i)
    {
        for (size_t j = 0; j < C; ++j)
            std::cout << mat.get()[i][j] << ' ';
        std::cout << '\n';
    }

    std::cout << '\n';
    std::cout << "Linearizando e multiplicando por 3";
    std::cout << '\n';

    // iteração linear com std::span (C++20)
    auto flat = matriz_span (mat, R, C);
    for (auto& x : flat) x *= 2;

    // impressão linear
    for (auto x : matriz_cspan (mat, R, C))
        std::cout << x << ' ';
    std::cout << '\n';

    // verificação: correspondência 1D <-> 2D
    for (size_t k = 0; k < flat.size(); ++k)
    {
        auto [i, j] = k_to_ij (k, C);
        assert (flat[k] == mat.get()[i][j]);
    }

    std::cout << '\n';
    std::cout << "retornando:";
    std::cout << '\n';
    for (size_t i = 0; i < R; ++i)
    {
        for (size_t j = 0; j < C; ++j)
            std::cout << mat.get()[i][j] << ' ';
        std::cout << '\n';
    }
}
