#ifndef MTK_DYNARRAY_HPP
#define MTK_DYNARRAY_HPP

#include "./array.hpp"

#include <iterator>
#include <new>

namespace mtk {
namespace impl_array {

template<class T
	,class = void>
struct is_fwd_iter : std::false_type { };

template<class T>
struct is_fwd_iter<T
	,std::void_t<
		std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<T>::iterator_category>, int>
	>> : std::true_type { };

} // namespace impl_array

template<class T
	,std::size_t N = dynamic_extent>
class dynarray
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
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr
	size_type
	extent = N;



	dynarray() noexcept :
		m_data(),
		m_size(0)
	{ }

	dynarray(size_type count, const T& value = T()) :
		m_data(_make_arr(count)),
		m_size(count)
	{
		auto ptr = this->data();
		while (count--) {
			_construct(ptr++, value);
		}
	}

	template<class FwdIter
		,std::enable_if_t<impl_array::is_fwd_iter<FwdIter>::value, int> = 0>
	dynarray(FwdIter first, FwdIter last) :
		m_data(_make_arr(std::distance(first, last))),
		m_size(m_data.size() / sizeof(T))
	{
		_construct_range(this->data(), first, last);
	}

	dynarray(std::initializer_list<T> ilist) :
		dynarray(ilist.begin(), ilist.end())
	{ }

	dynarray(const dynarray& other) :
		m_data(_make_arr(other.m_size)),
		m_size(other.m_size)
	{
		_construct_range(this->data(), other.begin(), other.end());
	}

	dynarray(dynarray&& other) noexcept :
		m_data(std::exchange(other.m_data, decltype(m_data)())),
		m_size(std::exchange(other.m_size, 0))
	{ }

	~dynarray()
	{
		this->clear();
	}

	dynarray&
	operator=(dynarray rhs) noexcept
	{
		this->swap(rhs);
		return *this;
	}

	dynarray&
	operator=(std::initializer_list<T> ilist)
	{
		this->assign(ilist);
		return *this;
	}

	template<class FwdIter>
	void
	assign(FwdIter first, FwdIter last)
	{
		const auto count = static_cast<size_type>(std::distance(first, last));
		if (this->capacity() >= count) {
			if (this->size() > count)
				this->resize(count);

			auto it = this->begin();
			const auto e = this->end();

			while (it != e) {
				*(it++) = *(first++);
			}
			_construct_range(this->end(), first, last);
			m_size = count;

		} else {
			*this = dynarray(first, last);
		}
	}

	void
	assign(std::initializer_list<T> ilist)
	{
		this->assign(ilist.begin(), ilist.end());
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
	data() { return reinterpret_cast<pointer>(m_data.data()); }

	const_pointer
	data() const { return reinterpret_cast<const_pointer>(m_data.data()); }



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

	reverse_iterator
	rbegin() { return reverse_iterator(this->end()); }

	const_reverse_iterator
	rbegin() const { return const_reverse_iterator(this->end()); }

	const_reverse_iterator
	crbegin() const { return this->rbegin(); }

	reverse_iterator
	rend() { return reverse_iterator(this->begin()); }

	const_reverse_iterator
	rend() const { const_reverse_iterator(this->begin()); }

	const_reverse_iterator
	crend() const { return this->rend(); }



	[[nodiscard]]
	bool
	empty() const { return (this->size() == 0); }

	size_type
	size() const { return m_size; }

	size_type
	max_size() const { return m_data.max_size() / sizeof(T); }

	void
	reserve(size_type size)
	{
		if constexpr (N == dynamic_extent) {
			if (size <= this->capacity())
				return;

			m_data.resize(size*sizeof(T));
		} else {
			(void)size;
		}
	}

	size_type
	capacity() const { return m_data.size() / sizeof(T); }

	void
	shrink_to_fit()
	{
		if constexpr (N == dynamic_extent) {
			if (this->size() == this->capacity())
				return;

			m_data.resize(this->size()*sizeof(T));
		}
	}



	void
	clear()
	{
		for (auto it = this->begin(), e = this->end(); it != e; ++it) {
			//Isildur!
			_destroy(it);
		}
		m_size = 0;
	}

	iterator
	insert(const_iterator pos, const T& value)
	{
		return this->emplace(pos, value);
	}

	iterator
	insert(const_iterator pos, T&& value)
	{
		return this->emplace(pos, std::move(value));
	}

	iterator
	insert(const_iterator pos, size_type count, const T& value)
	{
		const auto p = const_cast<iterator>(pos);
		if (count == 0)
			return p;

		this->_move_items_back(p, count);
		_construct_count(p, count, value);
		m_size += count;
		return p;
	}

	template<class FwdIter
		,std::enable_if_t<impl_array::is_fwd_iter<FwdIter>::value, int> = 0>
	iterator
	insert(const_iterator pos, FwdIter first, FwdIter last)
	{
		const auto p = const_cast<iterator>(pos);
		if (first == last)
			return p;

		const auto count = static_cast<size_type>(std::distance(last - first));
		this->_move_items_back(p, count);
		_construct_range(p, first, last);
		m_size += count;
		return p;
	}

	iterator
	insert(const_iterator pos, std::initializer_list<T> ilist)
	{
		return this->insert(pos, ilist.begin(), ilist.end());
	}

	template<class... Args>
	iterator
	emplace(const_iterator pos, Args&& ...args)
	{
		const auto p = const_cast<iterator>(pos);
		this->_move_items_back(p, 1);
		_construct(p, std::forward<Args>(args)...);
		++m_size;
		return p;
	}

	iterator
	erase(const_iterator pos)
	{
		const auto p = const_cast<iterator>(pos);
		_destroy(p);
		this->_move_items_fwd(p + 1, 1);
		--m_size;
		return p;
	}

	iterator
	erase(const_iterator first, const_iterator last)
	{
		const auto p = const_cast<iterator>(first);
		if (first == last)
			return p;

		const auto count = static_cast<size_type>(last - first);
		_destroy_count(p, count);
		this->_move_items_fwd(p + count, count);
		m_size -= count;
		return p;
	}

	void
	push_back(const T& value)
	{
		this->emplace_back(value);
	}

	void
	push_back(T&& value)
	{
		this->emplace_back(std::move(value));
	}

	template<class... Args>
	reference
	emplace_back(Args&& ...args)
	{
		const auto p = _construct(this->end(), std::forward<Args>(args)...);
		++m_size;
		return *p;
	}

	void
	pop_back()
	{
		_destroy(this->data() + --m_size);
	}

	void
	resize(size_type size, const T& val = T())
	{
		if (size <= this->size()) {
			const auto diff = (this->size() - size);
			_destroy_count(this->end() - diff, diff);
		} else {
			if constexpr (N == dynamic_extent) {
				m_data.resize(size*sizeof(T));
			}

			const auto diff = (size - this->size());
			_construct_count(this->end(), diff, val);
		}

		m_size = size;
	}

	void
	swap(dynarray& other) noexcept
	{
		m_data.swap(other.m_data);
		std::swap(m_size, other.m_size);
	}

private:
	using array_type = array<unsigned char,
		(N == dynamic_extent ? dynamic_extent : N*sizeof(T))>;

	static
	array_type
	_make_arr(std::size_t size)
	{
		(void)size;
		if constexpr (N == dynamic_extent)
			return array_type(size*sizeof(T));
		else
			return array_type();
	}

	template<class... Args>
	static
	pointer
	_construct(pointer pos, Args&& ...args)
	{
		return new (pos) T(std::forward<Args>(args)...);
	}

	template<class InputIter>
	static
	void
	_construct_range(pointer pos, InputIter first, InputIter last)
	{
		while (first != last) {
			_construct(pos++, *(first++));
		}
	}

	static
	void
	_construct_count(pointer pos, size_type count, const T& value)
	{
		for (size_type i = 0; i < count; ++i) {
			_construct(pos + i, value);
		}
	}

	void
	_move_items_back(iterator pos, size_type count)
	{
		const auto diff = static_cast<size_type>(this->end() - pos);
		for (size_type i = diff; i-- > 0;) {
			_construct(pos + count + i, std::move(pos[i]));
			_destroy(pos + i);
		}
	}

	void
	_move_items_fwd(iterator pos, size_type count)
	{
		const auto diff = static_cast<size_type>(this->end() - pos);
		for (size_type i = 0; i < diff; ++i) {
			_construct(pos - count + i, std::move(pos[i]));
			_destroy(pos + i);
		}
	}

	static
	void
	_destroy(pointer pos)
	{
		pos->~T();
	}

	static
	void
	_destroy_count(pointer pos, size_type count)
	{
		for (size_type i = 0; i < count; ++i) {
			_destroy(pos + i);
		}
	}

	array_type m_data;
	std::size_t m_size;
};

template<class T
	,class... Args>
dynarray(T&&, Args&&...) -> dynarray<std::common_type_t<std::decay_t<T>, std::decay_t<Args>...>, 1 + sizeof...(Args)>;



template<class T
	,std::size_t N>
void
swap(dynarray<T, N>& a, dynarray<T, N>& b) noexcept
{
	a.swap(b);
}



template<class T
	,std::size_t N>
constexpr
bool
operator==(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs)
{
	if (lhs.size() != rhs.size())
		return false;

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
operator!=(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs) { return !(lhs == rhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator<(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs)
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
operator>(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs) { return (rhs < lhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator<=(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs) { return !(rhs < lhs); }

template<class T
	,std::size_t N>
constexpr
bool
operator>=(const dynarray<T, N>& lhs, const dynarray<T, N>& rhs) { return !(lhs < rhs); }

} // namespace mtk

#endif
