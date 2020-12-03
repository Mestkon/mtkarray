## mtkarray

Requires C++17.

### mtk/array.hpp
`array<class T, std::size_t N = dynamic_extent>` <br>
A lightweight replacement for `std::array`, stripping out some of the includes by removing
`.at(size_type)`, `std::tuple` conformance and reverse iterators. Also doesn't provide `.fill(const T&)`
as `std::fill` is a thing.

One can also use the special variable `dynamic_extent` for a fixed runtime dependent array size.

Unifies constant size C arrays in a single template:
- `array<T, N>` -> replacement for T\[N\]
- `array<T, dynamic_extent>` -> replacement for new T\[n\]

Undefined behavior:
- Accessing elements outside of the array.
- `.front()` if `.empty()`
- `.back()` if `.empty()`
- Using iterators not belonging to the same instance.

Iterator invalidation:
- `array<T, N>`:
    - Never
- `array<T, dynamic_extent>`:
    - `.resize` if the size changes.
    - `operator=` always.
    - `swap` always.
