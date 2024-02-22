#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>

#include <memory>
#include <algorithm>

template <typename T>
class RawMemory {
public:
	RawMemory() = default;

	explicit RawMemory(size_t capacity);

	RawMemory(const RawMemory&) = delete;
	RawMemory& operator=(const RawMemory& rhs) = delete;

	RawMemory(RawMemory&& other) noexcept;

	RawMemory& operator=(RawMemory&& other) noexcept;

	~RawMemory();

	T* operator+(size_t offset) noexcept;

	const T* operator+(size_t offset) const noexcept;

	const T& operator[](size_t index) const noexcept;

	T& operator[](size_t index) noexcept;

	void Swap(RawMemory& other) noexcept;

	const T* GetAddress() const noexcept;

	T* GetAddress() noexcept;

	size_t Capacity() const;

private:
	static T* Allocate(size_t n) {
		return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
	}

	static void Deallocate(T* buf) noexcept {
		operator delete(buf);
	}

	T* buffer_ = nullptr;
	size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
	using iterator = T*;
	using const_iterator = const T*;

	Vector() = default;

	explicit Vector(size_t size);

	Vector(const Vector& other);

	Vector(Vector&& other) noexcept;

	Vector& operator=(const Vector& rhs);

	Vector& operator=(Vector&& rhs) noexcept;

	iterator begin() noexcept;

	iterator end() noexcept;

	const_iterator begin() const noexcept;

	const_iterator end() const noexcept;

	const_iterator cbegin() const noexcept;

	const_iterator cend() const noexcept;

	void Swap(Vector& other) noexcept;

	size_t Size() const noexcept;

	size_t Capacity() const noexcept;

	const T& operator[](size_t index) const noexcept;

	T& operator[](size_t index) noexcept;

	void Reserve(size_t new_capacity);

	void Resize(size_t new_size);

	template <typename... Args>
	T& EmplaceBack(Args&&... args);

	void PushBack(const T& value);

	void PushBack(T&& value);

	template <typename... Args>
	iterator Emplace(const_iterator pos, Args&&... args);

	iterator Insert(const_iterator pos, const T& value);

	iterator Insert(const_iterator pos, T&& value);

	void PopBack() noexcept;

	iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>);

	~Vector();

private:
	RawMemory<T> data_;
	size_t size_ = 0;

	void CopyOrMove(T* from, size_t amount, T* to);

	void CopyOrMove(const T* from, size_t amount, T* to);
};



//************************************************************************************
//            ,           =,           

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
	: buffer_(Allocate(capacity))
	, capacity_(capacity) {
}

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
	std::swap(buffer_, other.buffer_);
	std::swap(capacity_, other.capacity_);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& other) noexcept {
	std::swap(buffer_, other.buffer_);
	std::swap(capacity_, other.capacity_);
	return *this;
}

template <typename T>
RawMemory<T>::~RawMemory() {
	Deallocate(buffer_);
}

//------------------------------------------------------------------------------------
//                   ,          

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
	return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
	assert(index < capacity_);
	return buffer_[index];
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept {
	return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept {
	return buffer_;
}

//------------------------------------------------------------------------------------
//swap, capacity, + 

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
	std::swap(buffer_, other.buffer_);
	std::swap(capacity_, other.capacity_);
}


template <typename T>
size_t RawMemory<T>::Capacity() const {
	return capacity_;
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
	assert(offset <= capacity_);
	return buffer_ + offset;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
	return const_cast<RawMemory&>(*this) + offset;
}




//************************************************************************************
//            ,           =,           


template <typename T>
Vector<T>::Vector(size_t size)
	: data_(size)
	, size_(size)
{
	std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
	: data_(other.size_)
	, size_(other.size_)
{
	CopyOrMove(other.data_.GetAddress(), size_, data_.GetAddress());
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept
	: data_(std::move(other.data_))
	, size_(std::move(other.size_))
{
	other.size_ = 0;
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs) {
	if (this != &rhs) {
		if (rhs.size_ > Capacity()) {
			Vector rhs_copy(rhs);
			Swap(rhs_copy);
		}
		else {
			if (rhs.size_ < size_) {
				for (size_t i = 0; i < rhs.size_; ++i) {
					*(data_ + i) = *(rhs.data_ + i);
				}
				std::destroy_n(data_ + rhs.size_, size_ - rhs.size_);
			}
			else {
				for (size_t i = 0; i < size_; ++i) {
					*(data_ + i) = *(rhs.data_ + i);
				}
				std::uninitialized_copy_n(rhs.data_ + size_, rhs.size_ - size_, data_ + size_);
			}
			size_ = rhs.size_;
		}
	}
	return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept {
	if (this != &rhs) {
		if (rhs.size_ > Capacity()) {
			Vector rhs_copy(std::move(rhs));
			Swap(rhs_copy);
		}
	}
	return *this;
}

template <typename T>
Vector<T>::~Vector() {
	std::destroy_n(data_.GetAddress(), size_);
}

//------------------------------------------------------------------------------------
//         , []

template <typename T>
T* Vector<T>::begin() noexcept {
	return data_.GetAddress();
}

template <typename T>
T* Vector<T>::end() noexcept {
	return data_ + size_;
}

template <typename T>
const T* Vector<T>::begin() const noexcept {
	return reinterpret_cast<const T*>(data_.GetAddress());
}

template <typename T>
const T* Vector<T>::end() const noexcept {
	return reinterpret_cast<const T*>(data_ + size_);
}

template <typename T>
const T* Vector<T>::cbegin() const noexcept {
	return begin();
}

template <typename T>
const T* Vector<T>::cend() const noexcept {
	return end();
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
	return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
	assert(index < size_);
	return data_[index];
}

//------------------------------------------------------------------------------------
//      , capacity, swap

template <typename T>
size_t Vector<T>::Size() const noexcept {
	return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
	return data_.Capacity();
}

template <typename T>
void Vector<T>::Swap(Vector& other) noexcept {
	data_.Swap(other.data_);
	std::swap(size_, other.size_);
}

//------------------------------------------------------------------------------------
//reverse, resize

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
	if (new_capacity <= Capacity()) {
		return;
	}
	RawMemory<T> new_data(new_capacity);
	CopyOrMove(begin(), size_, new_data.GetAddress());
	data_.Swap(new_data);
	std::destroy_n(new_data.GetAddress(), size_);
}

template <typename T>
void Vector<T>::Resize(size_t new_size) {
	if (new_size < size_) {
		std::destroy_n(data_ + new_size, size_ - new_size);
	}
	else {
		if (size_ + new_size > data_.Capacity()) {
			Reserve(size_ == 0 ? new_size : size_ * 2);
		}
		std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
	}
	size_ = new_size;
}

//------------------------------------------------------------------------------------
//emplace/push back

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
	if (Capacity() == size_) {
		RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
		new (new_data + size_) T(std::forward<Args>(args)...);

		CopyOrMove(data_.GetAddress(), size_, new_data.GetAddress());
		data_.Swap(new_data);
		std::destroy_n(new_data.GetAddress(), size_);
	}
	else {
		new (data_ + size_) T(std::forward<Args>(args)...);
	}
	++size_;
	return *(data_ + (size_ - 1));
}

template <typename T>
void Vector<T>::PushBack(const T& value) {
	EmplaceBack(value);
}

template <typename T>
void Vector<T>::PushBack(T&& value) {
	EmplaceBack(std::move(value));
}

//------------------------------------------------------------------------------------
//emplace/insert

template <typename T>
template <typename... Args>
T* Vector<T>::Emplace(const_iterator pos, Args&&... args) {
	if (pos == end()) {
		EmplaceBack(std::forward<Args>(args)...);
		return data_ + size_ - 1;
	}

	size_t dist = pos - begin();
	bool after_pos = false;
	if (Capacity() > size_) {
		T copy_value(std::forward<Args>(args)...);
		new (data_ + size_) T(std::move(*(data_ + (size_ - 1))));
		std::move_backward(data_ + dist, end() - 1, end());
		data_[dist] = std::move(copy_value);
	}
	else {
		RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
		new (new_data + dist) T(std::forward<Args>(args)...);
		size_t second_dist = end() - pos;
		try {
			CopyOrMove(data_.GetAddress(), dist, new_data.GetAddress());
			after_pos = true;
			CopyOrMove(data_ + dist, second_dist, new_data + dist + 1);
		}
		catch (...) {
			std::destroy_at(new_data + dist);
			if (after_pos) {
				std::destroy_n(new_data.GetAddress(), dist);
			}
			throw;
		}
		data_.Swap(new_data);
		std::destroy_n(new_data.GetAddress(), size_);
	}
	++size_;
	return data_ + dist;
}

template <typename T>
T* Vector<T>::Insert(const_iterator pos, const T& value) {
	return Emplace(pos, value);
}

template <typename T>
T* Vector<T>::Insert(const_iterator pos, T&& value) {
	return Emplace(pos, std::move(value));
}

//------------------------------------------------------------------------------------
//erase, popback

template <typename T>
void Vector<T>::PopBack() noexcept {
	std::destroy_at(end() - 1);
	--size_;
}

template <typename T>
T* Vector<T>::Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
	size_t dist = pos - begin();
	std::move(data_ + dist + 1, data_ + size_, data_ + dist);
	PopBack();
	return data_ + dist;
}

//------------------------------------------------------------------------------------
//private

template <typename T>
void Vector<T>::CopyOrMove(T* from, size_t amount, T* to) {
	if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
		std::uninitialized_move_n(from, amount, to);
	}
	else {
		std::uninitialized_copy_n(from, amount, to);
	}
}

template <typename T>
void Vector<T>::CopyOrMove(const T* from, size_t amount, T* to) {
	std::uninitialized_copy_n(from, amount, to);
}
