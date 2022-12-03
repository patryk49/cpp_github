#pragma once

#include "Utils.hpp"


template<class T, size_t N>
struct FixedArray{
	constexpr
	T &operator [](size_t index) noexcept{ return this->data[index]; }
	
	constexpr
	const T &operator [](size_t index) const noexcept{ return this->data[index]; }

	typedef T ValueType;
	static constexpr size_t Size = N;
	static_assert(Size > 0, "this makes no sense");

	union{
		T data[N];
	};
};

template<class T, size_t N> static constexpr
size_t len(const FixedArray<T, N> &) noexcept{ return N; }

template<class T, size_t N> static constexpr
T *begin(const FixedArray<T, N> &arr) noexcept{ return arr.data; }
template<class T, size_t N> static constexpr
T *begin(FixedArray<T, N> &arr) noexcept{ return arr.data; }

template<class T, size_t N> static constexpr
T *beg(const FixedArray<T, N> &arr) noexcept{ return arr.data; }
template<class T, size_t N> static constexpr
T *beg(FixedArray<T, N> &arr) noexcept{ return arr.data; }

template<class T, size_t N> static constexpr
T *end(const FixedArray<T, N> &arr) noexcept{ return arr.data + N; }
template<class T, size_t N> static constexpr
T *end(FixedArray<T, N> &arr) noexcept{ return arr.data + N; }

template<class T, size_t N> constexpr bool needs_init<FixedArray<T, N>> = needs_init<T>;
template<class T, size_t N> constexpr bool needs_deinit<FixedArray<T, N>> = needs_deinit<T>;

template<class T, size_t N> static
void init(FixedArray<T, N> &arr) noexcept{
	for (size_t i=0; i!=N; i+=1) init(arr.data[i]);
}

template<class T, size_t C> static
void deinit(FixedArray<T, C> &arr) noexcept{
	if constexpr (needs_deinit<T>) for (size_t i=0; i!=arr.size; ++i) deinit(arr[i]);
}

template<class TL, size_t NL, class TR, size_t NR> static constexpr
bool operator ==(const FixedArray<TL, NL> &lhs, const FixedArray<TR, NR> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const TL *sent = lhs.data + lhs.size;
	const TR *J = rhs.data;
	for (const TL *I=lhs.data; I!=sent; ++I, ++J) if (*I != *J) return false;
	return true;
}

template<class TL, size_t NL, class TR, size_t NR> static constexpr
bool operator !=(const FixedArray<TL, NL> &lhs, const FixedArray<TR, NR> &rhs) noexcept{
	return !(lhs == rhs);
}





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
	UnsOfGivenSize<
		sizeof(T)>=sizeof(size_t) ? sizeof(size_t) : sizeof(size_t) ? sizeof(size_t)/2 : 1
	> size = 0;
};


template<class T, size_t C> static constexpr
size_t len(const FiniteArray<T, C> &arr) noexcept{ return arr.size; }

template<class T, size_t C> static constexpr
T *begin(const FiniteArray<T, C> &arr) noexcept{ return arr.data; }
template<class T, size_t C> static constexpr
T *begin(FiniteArray<T, C> &arr) noexcept{ return arr.data; }

template<class T, size_t C> static constexpr
T *beg(const FiniteArray<T, C> &arr) noexcept{ return arr.data; }
template<class T, size_t C> static constexpr
T *beg(FiniteArray<T, C> &arr) noexcept{ return arr.data; }

template<class T, size_t C> static constexpr
T *end(const FiniteArray<T, C> &arr) noexcept{ return (T *)arr.data + (size_t)arr.size; }
template<class T, size_t C> static constexpr
T *end(FiniteArray<T, C> &arr) noexcept{ return (T *)arr.data + (size_t)arr.size; }

template<class T, size_t C> static constexpr
size_t cap(const FiniteArray<T, C> &arr) noexcept{ return C; }

template<class T, size_t C> static constexpr
bool is_full(const FiniteArray<T, C> &arr) noexcept{ return arr.size == C; }

template<class T, size_t C> static constexpr
bool is_empty(const FiniteArray<T, C> &arr) noexcept{ return arr.size == 0; }

template<class T, size_t C> constexpr bool needs_init<FiniteArray<T, C>> = true;
template<class T, size_t C> constexpr bool needs_deinit<FiniteArray<T, C>> = needs_deinit<T>;

template<class T, size_t C> static
void init(FiniteArray<T, C> &arr) noexcept{ arr.size = 0; }

template<class T, size_t C> static
void deinit(FiniteArray<T, C> &arr) noexcept{
	if constexpr (needs_deinit<T>) for (size_t i=0; i!=arr.size; ++i) deinit(arr[i]);
}

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
void push_range(FiniteArray<T, C> &arr, Span<TR> range) noexcept{
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





#ifdef SP_ALLOCATORS
	template<class T, auto A = MallocAllocator<>{}>
#else
	template<class T,	auto A>
#endif
struct DynamicArray{
	T &operator [](size_t index) noexcept{
		return *((T *)this->.ptr + index);
	}
	
	const T &operator [](size_t index) const noexcept{
		return *((const T *)this->.ptr + index);
	}
	
	typedef T ValueType;
	constexpr static auto Allocator = A;
	
	T *ptr = nullptr;
	uint32_t size = 0;
	uint32_t cap = 0;
};


template<class T, auto A> static constexpr
size_t len(const DynamicArray<T, A> &arr) noexcept{ return arr.size; }

template<class T, auto A> static constexpr
T *begin(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.ptr; }

template<class T, auto A> static constexpr
T *beg(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.ptr; }

template<class T, auto A> static constexpr
T *end(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.ptr + arr.size; }

template<class T, auto A> static constexpr
size_t cap(const DynamicArray<T, A> &arr) noexcept{ return arr.cap / sizeof(T); }

template<class T, auto A> static constexpr
bool is_empty(const DynamicArray<T, A> &arr) noexcept{ return arr.size == 0; }

template<class T, auto AL, auto AR> static constexpr
bool operator ==(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const T *sent = (const T *)lhs.ptr + lhs.size;
	for (const T *I=(const T *)lhs.ptr, *J=(const T *)rhs.ptr; I!=sent; ++I, ++J)
		if (*I != *J) return false;
	return true;
}

template<class T, auto AL, auto AR> static constexpr
bool operator !=(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	return !(lhs == rhs);
}

template<class T, auto A> constexpr bool needs_init<DynamicArray<T, A>> = true;
template<class T, auto A> constexpr bool needs_deinit<DynamicArray<T, A>> = true;

template<class T, auto A> static
void deinit(DynamicArray<T, A> &arr) noexcept{
	if constexpr (needs_deinit<T>)
		for (size_t i=0; i!=arr.size; ++i) deinit(*((T *)arr.ptr+1));
	free(deref(A), Memblock{arr.ptr, arr.cap});
}

template<class T, auto A> static
bool resize(DynamicArray<T, A> &arr, size_t size) noexcept{
	if (arr.size < size){
		size_t bytes = size * sizeof(T);
		if (arr.cap < bytes){
			Memblock blk;
			blk = realloc(deref(A), Memblock{arr.ptr, arr.cap}, bytes);
			if (blk.ptr == nullptr) return true;
			arr.ptr = blk.ptr;
			arr.cap = blk.size;
		}

		if constexpr (needs_init<T>)
			for (T *I=(T *)arr.ptr+arr.size; I!=(T *)arr.ptr+size; ++I) init(*I);
	} else{
		if constexpr (needs_deinit<T>)
			for (T *I=(T *)arr.ptr+size; I!=(T *)arr.ptr+arr.size; ++I) deinit(*I);
	}
	
	arr.size = size;
	return false;
}

template<class T, auto A> static
void shrink_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
	if constexpr (needs_deinit<T>)
		for (T *I=(T *)arr.ptr+arr.size-amount; I!=(T *)arr.ptr+arr.size; ++I) deinit(*I);
	
	arr.size -= amount;
}

template<class T, auto A> static
bool expand_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
	size_t size = arr.size + amount;
	size_t bytes = size * sizeof(T);
	if (arr.cap < bytes){
		Memblock blk;
		blk = realloc(deref(A), Memblock{arr.ptr, arr.cap}, bytes);
		if (blk.ptr == nullptr) return true;
		arr.ptr = blk.ptr;
		arr.cap = blk.size;
	}

	if constexpr (needs_init<T>)
		for (T *I=(T *)arr.ptr+arr.size; I!=(T *)arr.ptr+size; ++I) init(*I);
	
	arr.size = size;
	return false;
}


template<class T, auto A> static
bool push(DynamicArray<T, A> &arr) noexcept{
	if (arr.size == arr.cap/sizeof(T)){
		Memblock blk;
		blk = realloc(
			deref(A), Memblock{(uint8_t *)arr.ptr, arr.cap},
			arr.size ? 2*arr.size*sizeof(T) : sizeof(T)<64 ? (64/sizeof(T))*sizeof(T) : sizeof(T)
		);
		if (blk.ptr == nullptr) return true;
		arr.ptr = (T *)blk.ptr;
		arr.cap = blk.size;
	}
	if constexpr (needs_init<T>) init(*((T *)arr.ptr+arr.size));

	arr.size += 1;
	return false;
}

template<class T, auto A, class TV> static
bool push_value(DynamicArray<T, A> &arr, const TV &value) noexcept{
	bool err = push(arr);
	if (!err) copy(*((T *)arr.ptr+arr.size-1), value);
	return err;
}

template<class T, auto A, class TV> static
bool push_range(DynamicArray<T, A> &arr, Span<TV> range) noexcept{
	size_t size = arr.size + range.size;
	if (arr.cap < size*sizeof(T)){
		Memblock blk;
		blk = realloc(deref(A), Memblock{arr.ptr, arr.cap}, size);
		if (blk.ptr == nullptr) return true;
		arr.ptr = blk.ptr;
		arr.cap = blk.size;
	}

	T *J = (T *)arr.ptr + arr.size;
	for (const TV *I=range.ptr; I!=range.ptr+range.size; ++I, ++J){
		if constexpr (needs_init<T>) init(*J);
		*J = *I;
	}
	arr.size = size;
	return false;
}

template<class T, auto A> static
void pop(DynamicArray<T, A> &arr) noexcept{ --arr.size; }

template<class T, auto A> static
T &&pop_value(DynamicArray<T, A> &arr) noexcept{ return (T &&)((T *)arr.ptr)[--arr.size]; }
