## mtkarray

### mtk/array.hpp
`array<class T, std::size_t N = dynamic_extent>` <br>
A lightweight replacement for `std::array`, stripping out some of the includes by removing
`.at(size_type)`, `std::tuple` conformance and reverse iterators. Also doesn't provide `.fill(const T&)`
as `std::fill` is a thing.

One can also use the special variable `dynamic_extent` for a fixed runtime dependent array size.

Unifies constant size C arrays in a single template:
- array<T, N> -> replacement for T\[N\]
- array<T, dynamic_extent> -> replacement for new T\[n\]

### mtk/dynarray.hpp
`dynarray<class T, std::size_t N = dynamic_extent>` <br>
A simple fixed capacity vector-like container which is allocated on the stack.
If `dynamic_extent` is used the capacity is no longer fixed and can be changed (heap allocated).
The difference between `dynarray<T, dynamic_extent>` and `std::vector` is that
the capacity is not automatically increased when needed, as this is the responsibility
of the user.
