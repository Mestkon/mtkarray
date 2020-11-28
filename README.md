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

### mtk/dynarray.hpp
`dynarray<class T, std::size_t N = dynamic_extent>` <br>
A simple fixed capacity vector-like container which is allocated on the stack.
If `dynamic_extent` is used the capacity is no longer fixed and can be changed (heap allocated).
The difference between `dynarray<T, dynamic_extent>` and `std::vector` is that
the capacity is not automatically increased when needed, as this is the responsibility
of the user, and therefore can be a bit faster in some cases.

Undefined behavior:
- Accessing elements outside of the array.
- `.front()` if `.empty()`
- `.back()` if `.empty()`
- Using iterators not belonging to the same instance.
- `emplace`ing elements into a container with no capacity left. `.reserve` capacity first.
- `insert`ing elements into a container with no capacity left. `.reserve` capacity first.
- `.push_back`ing elements into a container with no capacity left. `.reserve` capacity first.
- `.pop_back` an empty container.
- `.erase(.end())`
- Constructing a fixed-capacity `dynarray` with more elements than the capacity allows. Use `dynamic_extent` if increasing N is not feasable.
- `operator=(std::initializer_list)` for fixed-capacity `dynarray`s if the list contains more elements than capacity allows.

Iterator invalidation:
- `dynarray<T, dynamic_extent>`:
    - `.assign` if the capacity must be increased.
    - `.reserve` if the capacity changes.
    - `.resize` if the capacity changes.
    - `.shrink_to_fit` if the capacity changes.
    - `operator=` always.
    - `swap` always.
- all `dynarray`s:
    - `.insert` for iterators past the inserted elements.
    - `.emplace` for iterators past the emplaced element.
    - `.erase` for iterators past the erased elements.
    - `.push_back` only `.end()`
    - `.emplace_back` only `.end()`
    - `.pop_back` only `.end()`
