#pragma once

#include "Utils.hpp"
#include <math.h>

namespace sp{ // BEGINING OF NAMESPACE ///////////////////////////////////////////////////////////

template<class K>
constexpr size_t default_hash_impl(const K &key) noexcept{
	union{
		size_t hash;
		K key;
	} u{.key = key};

	if constexpr (sizeof(K) < sizeof(size_t))
		return u.hash & (SIZE_MAX >> (sizeof(K)*8-1));
	else
		return u.hash;
}

template<class K>
constexpr auto DefaultHash = default_hash_impl<K>;

template<class K>
constexpr K EmptyMark = (
	std::is_arithmetic_v<K>
	? std::is_signed_v<K> ? 1 << (sizeof(K)*8 - 1) : (K)-1
	: std::is_floating_point_v<K> ? (K)NAN
	: K{}
);

struct NoValue{};

template<class K, class V, size_t C, auto H = DefaultHash<K>, K M = EmptyMark<K>>
struct FiniteHashmap{
	SP_CI V &operator [](const K &key) noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}
	SP_CI const V &operator [](const K &key) const noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}

	FiniteHashmap() noexcept{ for (auto &it : keys) it = M; }


	typedef K KeyType;
	typedef V ValueType;
	constexpr static size_t Capacity = C;
	static_assert(std::has_single_bit(C), "capacity of open hash table must be a power of 2");
	static_assert(C > 0, "this makes no sense");

	union{
		K keys[C];
	};
	union{
		V values[C];
	};
};

template<class K, class V, size_t C, auto H, K M>
SP_CSI K empty_mark(const FiniteHashmap<K, V, C, H, M> &) noexcept{ return M; }


template<class K, class V, size_t C, auto H, K M>
SP_CSI size_t cap(const FiniteHashmap<K, V, C, H, M> &) noexcept{ return C; }

template<class K, class V, size_t C, auto H, K M>
SP_CSI bool contains(const FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return false;
		if (hm.keys[i] == key) return true;
	}
}

template<class K, class V, size_t C, auto H, K M>
SP_CSI V *get(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (V *)hm.values + i;
	}
}

template<class K, class V, size_t C, auto H, K M>
SP_CSI const V *get(const FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (const V *)hm.values + i;
	}
}

template<class K, class V, size_t C, auto H, K M>
SP_CSI bool set(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	size_t j = 0;
	while (hm.keys[i] != M && hm.keys[i] != key){
		i = (i+1) & (C-1);
		[[unlikely]] if (j == C-1) return true;
		++j;
	}
	if (hm.keys[i] == M) hm.keys[i] = key;
	if constexpr (sp::needs_init<V>) sp::init((V *)hm.values + i);
	return false;
}

template<class K, class V, size_t C, auto H, K M>
SP_CSI bool set(FiniteHashmap<K, V, C, H, M> &hm, const K &key, const V &value) noexcept{
	size_t i = H(key) & (C-1);
	size_t j = 0;
	while (hm.keys[i] != M && hm.keys[i] != key){
		i = (i+1) & (C-1);
		[[unlikely]] if (j == C-1) return true;
		++j;
	}
	if (hm.keys[i] == M) hm.keys[i] = key;
	if constexpr (sp::needs_init<V>) sp::init((V *)hm.values + i);
	hm.values[i] = value;	
	return false;
}

template<class K, class V, size_t C, auto H, K M>
SP_CSI bool remove(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	if (hm.keys[i] == M) return true;
	for (size_t j=i;;){
		hm.keys[i] = M;
		if constexpr (sp::needs_deinit<V>) sp::deinit((V *)hm.values + i);
		for (;;){
			j = (j+1) & (C-1);
			if (hm.keys[i] == M) return false;
			size_t k = H(hm.keys[j]) & (C-1);
			if (!(i<=j ? (i<k && k<=j) : (i<k || k<=j))) break;
		}
		hm.keys[i] = hm.keys[j];
		hm.values[i] = hm.values[j];
		i = j;
	}
}






/*
template<class T, class A>
struct DynamicArray{
	SP_CI T &operator [](size_t index) noexcept{
		return *((T *)this->data.ptr + index);
	}
	SP_CI const T &operator [](size_t index) const noexcept{
		return *((T *)this->data.ptr + index);
	}
	
	typedef T ValueType;
	
	Memblock data = {nullptr, 0};
	size_t size = 0;
	A *allocator = nullptr;
};

template<class T, class A>
SP_CSI T *beg(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.data.ptr; }

template<class T, class A>
SP_CSI T *end(const DynamicArray<T, A> &arr) noexcept{ return (T *)arr.data.ptr + arr.size; }

template<class T, class A>
SP_CSI size_t len(const DynamicArray<T, A> &arr) noexcept{ return arr.size; }

template<class T, class A>
SP_CSI size_t cap(const DynamicArray<T, A> &arr) noexcept{ return arr.data.size / sizeof(T); }

template<class T, class A>
SP_CSI T &front(DynamicArray<T, A> &arr) noexcept{ return *(T *)arr.data.ptr; }

template<class T, class A>
SP_CSI T &back(DynamicArray<T, A> &arr) noexcept{ return *((T *)arr.data.ptr+arr.size-1); }

template<class T, class A>
SP_CSI bool push(DynamicArray<T, A> &arr) noexcept{
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

template<class T, class A, class TR>
SP_CSI bool push_range(DynamicArray<T, A> &arr, Range<TR> range) noexcept{
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

template<class T, class A, class TV>
SP_CSI bool push_value(DynamicArray<T, A> &arr, const TV &value) noexcept{
	bool flag = push(arr);
	copy(*((T *)arr.data.ptr+arr.size-1), value);
	return flag;
}

template<class T, class A> SP_CSI void pop(DynamicArray<T, A> &arr) noexcept{ --arr.size; }

template<class T, class A> SP_CSI T &&pop_val(DynamicArray<T, A> &arr) noexcept{
	return (T &&)((T *)arr.data.ptr)[--arr.size];
}

template<class T, class A>
SP_CSI bool is_empty(const DynamicArray<T, A> &arr) noexcept{ return arr.size == 0; }

template<class T, class A>
SP_CSI bool resize(DynamicArray<T, A> &arr, size_t size) noexcept{
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

template<class T, class A>
SP_CSI void shrink_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
	if constexpr (needs_deinit<T>)
		for (T *I=(T *)arr.data.ptr+arr.size-amount; I!=(T *)arr.data.ptr+arr.size; ++I) deinit(*I);
	
	arr.size -= amount;
}

template<class T, class A>
SP_CSI bool expand_back(DynamicArray<T, A> &arr, size_t amount) noexcept{
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

template<class T, class AL, class AR>
SP_CSI bool operator ==(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const T *sent = (const T *)lhs.data.ptr + lhs.size;
	for (const T *I=(const T *)lhs.data.ptr, *J=(const T *)rhs.data.ptr; I!=sent; ++I, ++J)
		if (*I != *J) return false;
	return true;
}

template<class T, class AL, class AR>
SP_CSI bool operator !=(const DynamicArray<T, AL> &lhs, const DynamicArray<T, AR> &rhs) noexcept{
	return !(lhs == rhs);
}

template<class T, class A> constexpr bool needs_init<DynamicArray<T, A>> = true;
template<class T, class A> constexpr bool needs_deinit<DynamicArray<T, A>> = true;


template<class T, class A>
SP_CSI void deinit(DynamicArray<T, A> &arr) noexcept{
	if constexpr (needs_deinit<T>)
		for (size_t i=0; i!=arr.size; ++i) deinit(*((T *)arr.data.ptr+1));
	free(arr.allocator, arr.data);
}

*/
} // END OF NAMESPACE	///////////////////////////////////////////////////////////////////
