// g++ -std=c++17 -O2 -Wall -Wextra -pedantic matrix_uptr_v4.cpp -o app

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <cassert>

// ------------------- infra do seu make_matrix -------------------
template <typename T>
using MallocUPtrRows = std::unique_ptr<T*, decltype (&std::free)>;

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

    size_t header_size;
    if (mul_overflow (rows, sizeof (T*), header_size)) return {nullptr, &std::free};

    size_t elems, data_size;
    if (mul_overflow (rows, cols, elems)) return {nullptr, &std::free};
    if (mul_overflow (elems, sizeof (T), data_size)) return {nullptr, &std::free};

    const size_t align = alignof (T);
    size_t mis = header_size % align;
    size_t padding = mis ? (align - mis) : 0;

    size_t header_pad, total;
    if (add_overflow (header_size, padding, header_pad)) return {nullptr, &std::free};
    if (add_overflow (header_pad, data_size, total)) return {nullptr, &std::free};

    auto* raw = static_cast<uint8_t*> (std::malloc (total));
    if (!raw) return {nullptr, &std::free};

    MallocUPtrRows<T> row_table (reinterpret_cast<T**> (raw), &std::free);
    auto* data = reinterpret_cast<T*> (raw + header_pad);

    for (size_t i = 0; i < rows; ++i)
        row_table.get()[i] = data + i * cols;

    return row_table;
}
// ------------------- /infra do seu make_matrix -------------------


// -------- Range achatado C++17 --------
template <class T>
struct FlatRange
{
    T* first = nullptr;
    size_t n = 0;

    T* begin() const noexcept
    {
        return first;
    }
    T* end()   const noexcept
    {
        return first + n;
    }
    size_t size() const noexcept
    {
        return n;
    }
    T& operator[] (size_t i) const noexcept
    {
        return first[i];
    }
    explicit operator bool() const noexcept
    {
        return first != nullptr;
    }
};

// helpers de mapeamento índice
inline size_t ij_to_k (size_t i, size_t j, size_t cols)
{
    return i * cols + j;
}
inline void   k_to_ij (size_t k, size_t cols, size_t& i, size_t& j)
{
    i = k / cols;
    j = k % cols;
}

// cria um range mutável
template <typename T>
FlatRange<T> matriz_as_range (MallocUPtrRows<T> const& rows, size_t r, size_t c)
{
    if (!rows) return {};
    assert (rows.get()[0] != nullptr); // se veio do make_matrix, é contíguo
    return FlatRange<T> { rows.get()[0], r * c };
}

// cria um range const
template <typename T>
FlatRange<const T> matriz_as_crange (MallocUPtrRows<T> const& rows, size_t r, size_t c)
{
    if (!rows) return {};
    assert (rows.get()[0] != nullptr);
    return FlatRange<const T> { rows.get()[0], r * c };
}


// ------------------- exemplo de uso -------------------
int main()
{
    size_t rows = 4, cols = 3;
    auto mat = make_matrix<int> (rows, cols);
    if (!mat)
    {
        std::cerr << "falha alocando matriz\n";
        return EXIT_FAILURE;
    }

    // preenche via acesso 2D tradicional
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            mat.get()[i][j] = int (10*i + j);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
            std::cout << mat.get()[i][j] << ' ';
        std::cout << '\n';
    }

    std::cout << '\n';
    std::cout << "Linearizando e multiplicando por 3";
    std::cout << '\n';

    // itera como vetor (range achatado)
    auto flat = matriz_as_range (mat, rows, cols);
    for (auto& x : flat) x *= 3;  // modifica todos os elementos

    // imprime linearmente
    for (auto const& x : matriz_as_crange (mat, rows, cols))
        std::cout << x << ' ';
    std::cout << '\n';

    // também dá para indexar como vetor:
    for (size_t k = 0; k < flat.size(); ++k)
    {
        size_t i, j;
        k_to_ij (k, cols, i, j);
        // checagem: flat[k] == mat[i][j]
        assert (flat[k] == mat.get()[i][j]);
    }

    std::cout << '\n';
    std::cout << "retornando:";
    std::cout << '\n';
    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
            std::cout << mat.get()[i][j] << ' ';
        std::cout << '\n';
    }
}
