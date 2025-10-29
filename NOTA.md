Boa! Se você **não** quiser usar `malloc` e ainda assim quer **alocação linear (contígua)**, as melhores opções em C++ moderno são:

## 1) `std::vector<T>` (+ `std::span`/indexador) — simples e segura

```cpp
#include <vector>
#include <span>
#include <cstddef>

template<class T>
struct Matrix {
    std::size_t rows, cols;
    std::vector<T> data;                  // linear

    Matrix(std::size_t r, std::size_t c) : rows(r), cols(c), data(r*c) {}

    T&       at(std::size_t i, std::size_t j)       { return data[i*cols + j]; }
    const T& at(std::size_t i, std::size_t j) const { return data[i*cols + j]; }

    std::span<T>       flat()       { return {data.data(), data.size()}; }
    std::span<const T> flat() const { return {data.data(), data.size()}; }
};
```

* Contíguo, RAII, sem custom deleter, cresce sozinho se precisar.
* Iterar linear: `for (auto& x : M.flat()) x *= 2;`

## 2) `std::mdspan` (C++23) — *view* 2D sobre buffer linear

```cpp
#include <mdspan>
#include <vector>

using ext = std::extents<std::size_t, std::dynamic_extent, std::dynamic_extent>;

template<class T>
struct MatrixView {
    std::vector<T> buf;
    std::mdspan<T, ext> view;  // não-dono

    MatrixView(std::size_t r, std::size_t c)
      : buf(r*c), view(buf.data(), r, c) {}
};
```

* Usa `mdspan` para `view` 2D ergonômica; armazenamento ainda é `vector` (linear).
* Acesso: `mv.view(i,j)`; varredura linear: `for (auto& x : mv.buf) ...`

*(Se estiver em C++20, dá para usar o backport oficial do `mdspan`.)*

## 3) `std::unique_ptr<T[]>` — leve, sem dependências de STL pesada

```cpp
#include <memory>
#include <span>

template<class T>
struct MatrixUPtr {
    std::size_t rows, cols;
    std::unique_ptr<T[]> data;  // linear

    MatrixUPtr(std::size_t r, std::size_t c)
      : rows(r), cols(c), data(std::make_unique<T[]>(r*c)) {}

    T&       at(std::size_t i, std::size_t j)       { return data[i*cols + j]; }
    const T& at(std::size_t i, std::size_t j) const { return data[i*cols + j]; }

    std::span<T>       flat()       { return {data.get(), rows*cols}; }
    std::span<const T> flat() const { return {data.get(), rows*cols}; }
};
```

* Também contíguo. Bom quando você não precisa de *resize*.

## 4) Bibliotecas de álgebra (contíguas por padrão, row-major)

* **Eigen** (`Matrix<T, Dynamic, Dynamic, RowMajor>`), **Blaze**, **xtensor**.
  Ganham em expressividade e otimizações, mantendo armazenamento linear.

---

### Qual eu usaria?

* **#1 `std::vector` + `span`**: 90% dos casos — simples, segura, rápida.
* Precisa só de *view* 2D elegante? **#2 `mdspan`** em cima do `vector`.
* Ambiente mínimo/embedded? **#3 `unique_ptr<T[]>`**.

Se quiser, eu te gero um **snippet UltiSnips (C++)** para a opção que preferir (vector/mdspan/unique_ptr).

