#ifndef MTK_ARRAY_DYNAMIC_EXTENT_HPP
#define MTK_ARRAY_DYNAMIC_EXTENT_HPP

#include "./array.hpp"

#include <memory>

namespace mtk {

template<class T
	,class Alloc>
class array<T, dynamic_extent, Alloc>
{
public:
	using _alloc = std::conditional_t<std::is_same_v<void, Alloc>, std::allocator<T>, Alloc>;

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using allocator_type = typename std::allocator_traits<_alloc>::template rebind_alloc<T>;

	using _alloc_trait = std::allocator_traits<allocator_type>;

	static constexpr
	std::size_t
	extent = dynamic_extent;



	array() :
		array(0)
	{ }

	explicit
	array(const allocator_type& alloc) :
		array(0, alloc)
	{ }

	explicit
	array(size_type size, const allocator_type& alloc = allocator_type()) :
		m_storage(alloc)
	{
		m_storage.size = size;

		if (size == 0) {
			m_storage.data = nullptr;
		} else {

			m_storage.data = _alloc_trait::allocate(m_storage.alloc(), size);

			auto* ptr = m_storage.data;
			while (size--) {
				_alloc_trait::construct(m_storage.alloc(), ptr++);
			}
		}
	}

	array(std::initializer_list<value_type> ilist, const allocator_type& alloc = allocator_type()) :
		array(ilist.size(), alloc)
	{
		this->_copy_range(ilist.begin(), ilist.end(), std::false_type());
	}

	array(const array& other) :
		array(other, other.get_allocator())
	{ }

	array(const array& other, const allocator_type& alloc) :
		array(other.size(), alloc)
	{
		this->_copy_range(other.begin(), other.end(), std::false_type());
	}

	array(array&& other) noexcept :
		array(other.get_allocator())
	{
		this->_simple_swap(other);
	}

	array(array&& other, const allocator_type& alloc)
	noexcept(_alloc_trait::is_always_equal) :
		array(alloc)
	{
		if constexpr (_alloc_trait::is_always_equal) {
			this->_simple_swap(other);
		} else if (alloc == other.get_allocator()) {
			this->_simple_swap(other);
		} else {
			array tmp(other.size(), alloc);
			tmp._copy_range(other.begin(), other.end(), std::true_type());
			this->_simple_swap(tmp);
		}
	}

	~array() noexcept
	{
		if (m_storage.size == 0)
			return;

		auto* ptr = m_storage.data;
		auto size = m_storage.size;
		while (size--) {
			_alloc_trait::destroy(m_storage.alloc(), ptr++);
		}
		_alloc_trait::deallocate(m_storage.alloc(), m_storage.data, m_storage.size);
	}

//	array&
//	operator=(array rhs) noexcept
//	{
//		this->swap(rhs);
//		return *this;
//	}

	array&
	operator=(const array& rhs)
	{
		if constexpr (_alloc_trait::propagate_on_container_copy_assignment::value) {
			array cp = rhs;
			array tmp(std::move(*this));
			m_storage = cp.get_allocator();
			this->_simple_swap(cp);
		} else {
			array cp(rhs, this->get_allocator());
			array tmp(std::move(*this));
			this->_simple_swap(cp);
		}
		return *this;
	}

	array&
	operator=(array&& rhs)
	noexcept(_alloc_trait::is_always_equal)
	{
		if constexpr (_alloc_trait::propagate_on_container_move_assignment::value) {
			array cp = std::move(rhs);
			array tmp(std::move(*this));
			m_storage = cp.get_allocator();
			this->_simple_swap(cp);
		} else {
			array cp(std::move(rhs), this->get_allocator());
			array tmp(std::move(*this));
			this->_simple_swap(cp);
		}
		return *this;
	}

	allocator_type
	get_allocator() const { return m_storage; }



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
	data() { return m_storage.data; }

	const_pointer
	data() const { return m_storage.data; }



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
	size() const { return m_storage.size; }

	size_type
	max_size() const { return _alloc_trait::max_size(); }

	void
	resize(std::size_t size)
	{
		if (size == this->size())
			return;

		array new_arr(size, this->get_allocator());
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
		using std::swap;
		if constexpr (_alloc_trait::propagate_on_container_swap::value)
			swap(static_cast<allocator_type&>(m_storage), static_cast<allocator_type&>(other.m_storage));

		swap(m_storage.data, other.m_storage.data);
		swap(m_storage.size, other.m_storage.size);
	}

private:
	template<class Iter
		,class MoveTrait>
	void
	_copy_range(Iter first, Iter last, MoveTrait)
	{
		auto it = this->begin();
		while (first != last) {
			if constexpr (MoveTrait::value)
				*(it++) = std::move(*(first++));
			else
				*(it++) = *(first++);
		}
	}

	void
	_simple_swap(array& other) {
		m_storage.data = std::exchange(other.m_storage.data, nullptr);
		m_storage.size = std::exchange(other.m_storage.size, 0);
	}

	struct _storage :
		allocator_type
	{
		_storage(const allocator_type& a) :
			allocator_type(a)
		{ }

		_storage&
		operator=(const allocator_type& a)
		{
			static_cast<allocator_type&>(*this) = a;
			return *this;
		}

		allocator_type&
		alloc() { return *this; }

		pointer data;
		size_type size;
	} m_storage;
};

} // namespace mtk

#endif
