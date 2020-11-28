#ifndef MTK_ARRAY_HPP
#define MTK_ARRAY_HPP

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <utility>
#include <type_traits>

namespace mtk {
namespace impl_array {

template<class T
	,std::size_t N>
struct array_traits
{
	using align_type = std::conditional_t<
		std::is_same_v<T, char> || std::is_same_v<T, unsigned char>,
		std::max_align_t, T[N]>;
	using storage_type = T[N];
};

template<class T>
struct array_traits<T, 0>
{
	struct storage_type { };
	using align_type = storage_type;
};

} // namespace impl_array



inline constexpr
std::size_t
dynamic_extent = std::numeric_limits<std::size_t>::max();

template<class T
	,std::size_t N = dynamic_extent>
class array
{
public:
	using _trait = impl_array::array_traits<T, N>;

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;

	static constexpr
	std::size_t
	extent = N;



	alignas(typename _trait::align_type)
	typename _trait::storage_type _data;



	constexpr
	reference
	operator[](size_type idx) { return *(this->begin() + idx); }

	constexpr
	const_reference
	operator[](size_type idx) const { return *(this->begin() + idx); }

	constexpr
	reference
	front() { return *this->begin(); }

	constexpr
	const_reference
	front() const { return *this->begin(); }

	constexpr
	reference
	back() { return *(this->end() - 1); }

	constexpr
	const_reference
	back() const { return *(this->end() - 1); }

	constexpr
	pointer
	data() { return reinterpret_cast<pointer>(&_data); }

	constexpr
	const_pointer
	data() const { return reinterpret_cast<const_pointer>(&_data); }



	constexpr
	iterator
	begin() { return this->data(); }

	constexpr
	const_iterator
	begin() const { return this->data(); }

	constexpr
	const_iterator
	cbegin() const { return this->begin(); }

	constexpr
	iterator
	end() { return (this->data() + this->size()); }

	constexpr
	const_iterator
	end() const { return (this->data() + this->size()); }

	constexpr
	const_iterator
	cend() const { return this->end(); }



	[[nodiscard]]
	constexpr
	bool
	empty() const { return (this->size() == 0); }

	constexpr
	size_type
	size() const { return N; }

	constexpr
	size_type
	max_size() const { return this->size(); }



	constexpr
	void
	swap(array& other) noexcept { std::swap(*this, other); }
};

template<class T>
class array<T, dynamic_extent>
{
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;

	static constexpr
	std::size_t
	extent = dynamic_extent;



	array() noexcept :
		array(0)
	{ }

	explicit
	array(size_type size) :
		m_data(size == 0 ? nullptr : new T[size]),
		m_size(size)
	{ }

	array(std::initializer_list<value_type> ilist) :
		array(ilist.size())
	{
		auto first = ilist.begin();
		const auto last = ilist.end();
		auto it = this->begin();
		while (first != last) {
			*(it++) = *(first++);
		}
	}

	array(const array& other) :
		array(other.begin(), other.end())
	{ }

	array(array&& other) noexcept :
		m_data(std::exchange(other.m_data, nullptr)),
		m_size(std::exchange(other.m_size, 0))
	{ }

	~array() noexcept { delete[] m_data; }

	array&
	operator=(array rhs) noexcept
	{
		this->swap(rhs);
		return *this;
	}



	reference
	operator[](size_type idx) { return *(this->begin() + idx); }

	const_reference
	operator[](size_type idx) const { return *(this->begin() + idx); }

	reference
	front() { return *this->begin(); }

	const_reference
	front() const { return *this->begin(); }

	reference
	back() { return *(this->end() - 1); }

	const_reference
	back() const { return *(this->end() - 1); }

	pointer
	data() { return m_data; }

	const_pointer
	data() const { return m_data; }



	iterator
	begin() { return this->data(); }

	const_iterator
	begin() const { return this->data(); }

	const_iterator
	cbegin() const { return this->begin(); }

	iterator
	end() { return (this->data() + this->size()); }

	const_iterator
	end() const { return (this->data() + this->size()); }

	const_iterator
	cend() const { return this->end(); }



	[[nodiscard]]
	bool
	empty() const { return (this->size() == 0); }

	size_type
	size() const { return m_size; }

	size_type
	max_size() const { return std::numeric_limits<size_type>::max(); }

	void
	resize(std::size_t size)
	{
		if (size == this->size())
			return;

		array new_arr(size);
		auto first = this->begin();
		const auto last = this->end();
		auto it = new_arr.begin();
		const auto end = new_arr.end();
		while ((first != last) && (it != end)) {
			if constexpr (std::is_nothrow_move_assignable_v<value_type>)
				*(it++) = std::move(*(first++));
			else
				*(it++) = *(first++);
		}

		this->swap(new_arr);
	}



	void
	swap(array& other) noexcept
	{
		std::swap(m_data, other.m_data);
		std::swap(m_size, other.m_size);
	}

private:
	pointer m_data;
	size_type m_size;
};

template<class T
	,class... Args>
array(T&&, Args&&...) -> array<std::common_type_t<std::decay_t<T>, std::decay_t<Args>...>, 1 + sizeof...(Args)>;



template<class T
	,std::size_t N>
constexpr
void
swap(array<T, N>& a, array<T, N>& b) noexcept
{
	a.swap(b);
}



template<class T
	,std::size_t N>
constexpr
bool
operator==(const array<T, N>& lhs, const array<T, N>& rhs)
{
	if constexpr (N == dynamic_extent) {
		if (lhs.size() != rhs.size())
			return false;
	}

	auto it = lhs.begin();
	const auto end = lhs.end();
	auto first = rhs.begin();
	while (it != end) {
		if (*(it++) != *(first++))
			return false;
	}
	return true;
}

template<class T
	,std::size_t N>
constexpr
bool
operator!=(const array<T, N>& lhs, const array<T, N>& rhs) { return !(lhs == rhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator<(const array<T, N>& lhs, const array<T, N>& rhs)
{
	auto it = lhs.begin();
	const auto end = lhs.end();
	auto first = rhs.begin();
	const auto last = rhs.end();

	while ((it != end) && (first != last)) {
		if (*it < *first)
			return true;
		else if (*it > *first)
			return false;

		++it;
		++first;
	}

	if (first != last)
		return true;
	else
		return false;
}
template<class T
	,std::size_t N>
constexpr
bool
operator>(const array<T, N>& lhs, const array<T, N>& rhs) { return (rhs < lhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator<=(const array<T, N>& lhs, const array<T, N>& rhs) { return !(rhs < lhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator>=(const array<T, N>& lhs, const array<T, N>& rhs) { return !(lhs < rhs); }

} // namespace mtk


#endif
