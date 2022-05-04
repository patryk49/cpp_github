#pragma once

#include "Utils.hpp"



template<class T, size_t C>
struct FiniteArray{
	constexpr
	T &operator [](size_t index) noexcept{ return this->data[index]; }
	
	constexpr
	const T &operator [](size_t index) const noexcept{ return this->data[index]; }

	typedef T ValueType;
	static constexpr size_t Capacity = C;
	static_assert(Capacity > 0, "this makes no sense");

	union{
		T data[C];
	};
	UIntOfGivenSize<
		sizeof(T)>=sizeof(size_t) ? sizeof(size_t) : sizeof(size_t) ? sizeof(size_t)/2 : 1
	> size = 0;
};


template<class T, size_t C> static constexpr
size_t len(const FiniteArray<T, C> &arr) noexcept{ return arr.size; }

template<class T, size_t C> static constexpr
T *beg(const FiniteArray<T, C> &arr) noexcept{ return arr.data; }

template<class T, size_t C> static constexpr
T *end(const FiniteArray<T, C> &arr) noexcept{ return arr.data + (size_t)arr.size; }

template<class T, size_t C> static constexpr
size_t cap(const FiniteArray<T, C> &arr) noexcept{ return C; }

template<class T, size_t C> static constexpr
bool is_full(const FiniteArray<T, C> &arr) noexcept{ return arr.size == C; }

template<class T, size_t C> static constexpr
bool is_empty(const FiniteArray<T, C> &arr) noexcept{ return arr.size == 0; }


template<class T, size_t C> static
void resize(FiniteArray<T, C> &arr, size_t size) noexcept{
	if constexpr (needs_init<T>)
		if (arr.size < size) for (T *I=arr.data+arr.size; I!=arr.data+size; ++I) init(*I);
	if constexpr (needs_deinit<T>)
		if (arr.size > size) for (T *I=arr.data+arr.size; --I!=arr.data+size-1;) deinit(*I);
	
	arr.size = size;
}

template<class T, size_t C> static
void expand_back(FiniteArray<T, C> &arr, size_t amount) noexcept{
	if constexpr (needs_init<T>)
		for (T *I=arr.data+arr.size; I!=arr.data+arr.size+amount; ++I) init(*I);
	
	arr.size += amount;
}

template<class T, size_t C> static
void shrink_back(FiniteArray<T, C> &arr, size_t amount) noexcept{
	if constexpr (needs_deinit<T>)
		for (T *I=arr.data+arr.size; --I!=arr.data+arr.size-amount-1;) deinit(*I);
	
	arr.size -= amount;
}


template<class T, size_t C> static
void push(FiniteArray<T, C> &arr) noexcept{
	if constexpr (needs_init<T>) init(arr.data[arr.size]);
	++arr.size;
}

template<class T, size_t C, class TV> static
void push_value(FiniteArray<T, C> &arr, const TV &value) noexcept{
	if constexpr (needs_init<T>) init(arr.data[arr.size]);
	copy(arr.data[(size_t)arr.size], value);
	++arr.size;
}

template<class T, size_t C, class TR> static
void push_range(FiniteArray<T, C> &arr, Range<TR> range) noexcept{
	T *J = arr.data + arr.size;
	for (TR *I=range.ptr; I!=range.ptr+range.size; ++I, ++J){
		if constexpr (needs_init<T>) init(*J);
		*J = *I;
	}
	arr.size += range.size;
}

template<class T, size_t C> static
void pop(FiniteArray<T, C> &arr) noexcept{
	--arr.size;
	if constexpr (needs_deinit<T>) deinit(arr.data[arr.size]);
}

template<class T, size_t C> static
T &&pop_value(FiniteArray<T, C> &arr) noexcept{
	--arr.size;
	return (T &&)arr.data[arr.size];
}


template<class TL, size_t CL, class TR, size_t CR> static constexpr
bool operator ==(const FiniteArray<TL, CL> &lhs, const FiniteArray<TR, CR> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const TL *sent = lhs.data + lhs.size;
	const TR *J = rhs.data;
	for (const TL *I=lhs.data; I!=sent; ++I, ++J) if (*I != *J) return false;
	return true;
}

template<class TL, size_t CL, class TR, size_t CR> static constexpr
bool operator !=(const FiniteArray<TL, CL> &lhs, const FiniteArray<TR, CR> &rhs) noexcept{
	return !(lhs == rhs);
}


template<class T, size_t C> constexpr bool needs_init<FiniteArray<T, C>> = true;
template<class T, size_t C> constexpr bool needs_deinit<FiniteArray<T, C>> = needs_deinit<T>;

template<class T, size_t C> static
void init(FiniteArray<T, C> &arr) noexcept{ arr.size = 0; }

template<class T, size_t C> static
void deinit(FiniteArray<T, C> &arr) noexcept{
	if constexpr (needs_deinit<T>) for (size_t i=0; i!=arr.size; ++i) deinit(arr[i]);
}






template<class T, class A>
struct DynamicArray{
	constexpr
	T &operator [](size_t index) noexcept{
		return *((T *)this->data.ptr + index);
	}
	
	constexpr
	const T &operator [](size_t index) const noexcept{
		return *((const T *)this->data.ptr + index);
	}
	
	typedef T ValueType;
	
	Memblock data = {nullptr, 0};
	size_t size = 0;
	A *allocator = nullptr;
};


template<class T, class A> static constexpr
size_t len(const DynamicArray<T, A> &arr) noexcept{ return arr.size; }

template<class T, class A> static constexpr
T *beg(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.data.ptr; }

template<class T, class A> static constexpr
T *end(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.data.ptr + arr.size; }

template<class T, class A> static constexpr
size_t cap(const DynamicArray<T, A> &arr) noexcept{ return arr.data.size / sizeof(T); }

template<class T, class A> static constexpr
bool is_empty(const DynamicArray<T, A> &arr) noexcept{ return arr.size == 0; }


template<class T, class A> static
bool resize(DynamicArray<T, A> &arr, size_t size) noexcept{
	if (arr.size < size){
		size_t bytes = size * sizeof(T);
		if (arr.data.size < bytes){
			Memblock blk;
			if constexpr (A::Alignment)
				blk = realloc(*arr.allocator, arr.data, bytes);
			else
				blk = realloc(*arr.allocator, arr.data, bytes, alignof(T));
			if (blk.ptr == nullptr) return true;
			arr.data = blk;	
		}

		if constexpr (needs_init<T>)
			for (T *I=(T *)arr.data.ptr+arr.size; I!=(T *)arr.data.ptr+size; ++I) init(*I);
	} else{
		if constexpr (needs_deinit<T>)
			for (T *I=(T *)arr.data.ptr+size; I!=(T *)arr.data.ptr+arr.size; ++I) deinit(*I);
	}
	
	arr.size = size;
	return false;
}

template<class T, class A> static
void shrink_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
	if constexpr (needs_deinit<T>)
		for (T *I=(T *)arr.data.ptr+arr.size-amount; I!=(T *)arr.data.ptr+arr.size; ++I) deinit(*I);
	
	arr.size -= amount;
}

template<class T, class A> static
bool expand_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
	size_t size = arr.size + amount;
	size_t bytes = size * sizeof(T);
	if (arr.data.size < bytes){
		Memblock blk;
		if constexpr (A::Alignment)
			blk = realloc(*arr.allocator, arr.data, bytes);
		else
			blk = realloc(*arr.allocator, arr.data, bytes, alignof(T));
		if (blk.ptr == nullptr) return true;
		arr.data = blk;	
	}

	if constexpr (needs_init<T>)
		for (T *I=(T *)arr.data.ptr+arr.size; I!=(T *)arr.data.ptr+size; ++I) init(*I);
	
	arr.size = size;
	return false;
}


template<class T, class A> static
bool push(DynamicArray<T, A> &arr) noexcept{
	if (arr.size == arr.data.size/sizeof(T)){
		Memblock blk;
		if constexpr (A::Alignment)
			blk = realloc(
				*arr.allocator, arr.data,
				arr.size ? 2*arr.size*sizeof(T) : sizeof(T)<64 ? (64/sizeof(T))*sizeof(T) : sizeof(T)
			);
		else
			blk = realloc(
				*arr.allocator, arr.data,
				arr.size ? 2*arr.size*sizeof(T) : sizeof(T)<64 ? (64/sizeof(T))*sizeof(T) : sizeof(T),
				alignof(T)
			);
		if (blk.ptr == nullptr) return true;
		arr.data = blk;
	}
	if constexpr (needs_init<T>) init(*((T *)arr.data.ptr+arr.size));

	++arr.size;
	return false;
}

template<class T, class A, class TV> static
bool push_value(DynamicArray<T, A> &arr, const TV &value) noexcept{
	bool flag = push(arr);
	copy(*((T *)arr.data.ptr+arr.size-1), value);
	return flag;
}

template<class T, class A, class TR> static
bool push_range(DynamicArray<T, A> &arr, Range<TR> range) noexcept{
	size_t size = arr.size + range.size;
	if (arr.data.size < size*sizeof(T)){
		Memblock blk;
		if constexpr (A::Alignment)
			blk = realloc(*arr.allocator, arr.data, size);
		else
			blk = realloc(*arr.allocator, arr.data, size, alignof(T));
		if (blk.ptr == nullptr) return true;
		arr.data = blk;	
	}

	T *J = (T *)arr.data.ptr + arr.size;
	for (const TR *I=range.ptr; I!=range.ptr+range.size; ++I, ++J){
		if constexpr (needs_init<T>) init(*J);
		*J = *I;
	}
	arr.size = size;
	return false;
}

template<class T, class A> static
void pop(DynamicArray<T, A> &arr) noexcept{ --arr.size; }

template<class T, class A> static
T &&pop_value(DynamicArray<T, A> &arr) noexcept{ return (T &&)((T *)arr.data.ptr)[--arr.size]; }


template<class T, class AL, class AR> static constexpr
bool operator ==(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const T *sent = (const T *)lhs.data.ptr + lhs.size;
	for (const T *I=(const T *)lhs.data.ptr, *J=(const T *)rhs.data.ptr; I!=sent; ++I, ++J)
		if (*I != *J) return false;
	return true;
}

template<class T, class AL, class AR> static constexpr
bool operator !=(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	return !(lhs == rhs);
}


template<class T, class A> constexpr bool needs_init<DynamicArray<T, A>> = true;
template<class T, class A> constexpr bool needs_deinit<DynamicArray<T, A>> = true;

template<class T, class A> static
void deinit(DynamicArray<T, A> &arr) noexcept{
	if constexpr (needs_deinit<T>)
		for (size_t i=0; i!=arr.size; ++i) deinit(*((T *)arr.data.ptr+1));
	free(arr.allocator, arr.data);
}

