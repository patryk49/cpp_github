#pragma once

#include "Utils.hpp"



template<class K> static constexpr
size_t _impl_DefaultHash(const K &key) noexcept{
	union{
		size_t hash;
		K key;
	} u{.key = key};

	if constexpr (sizeof(K) < sizeof(size_t))
		return u.hash & (SIZE_MAX >> (sizeof(K)*8-1));
	else
		return u.hash;
}

template<class K> constexpr
auto DefaultHash = _impl_DefaultHash<K>;


template<class K>
constexpr K EmptyMark = (
	std::is_arithmetic_v<K>
	? std::is_signed_v<K> ? 1 << (sizeof(K)*8 - 1) : (K)-1
	: std::is_floating_point_v<K> ? (K)NAN
	: K{}
);





template<class K, class V, size_t C, auto H = DefaultHash<K>, K M = EmptyMark<K>>
struct FiniteHashmap{
	constexpr
	V &operator [](const K &key) noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}

	constexpr
	const V &operator [](const K &key) const noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}

	constexpr
	FiniteHashmap() noexcept{ for (auto &it : keys) it = M; }


	typedef K KeyType;
	typedef V ValueType;
	static constexpr size_t Capacity = C;
	static_assert(std::has_single_bit(C), "capacity of open hash table must be a power of 2");
	static_assert(C > 0, "this makes no sense");

	union{
		K keys[C];
	};
	union{
		V values[C];
	};
};


template<class K, class V, size_t C, auto H, K M> static constexpr
K empty_mark(const FiniteHashmap<K, V, C, H, M> &) noexcept{ return M; }


template<class K, class V, size_t C, auto H, K M> static constexpr
size_t cap(const FiniteHashmap<K, V, C, H, M> &) noexcept{ return C; }

template<class K, class V, size_t C, auto H, K M>
bool contains(const FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return false;
		if (hm.keys[i] == key) return true;
	}
}

template<class K, class V, size_t C, auto H, K M> static constexpr
V *get(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (V *)hm.values + i;
	}
}

template<class K, class V, size_t C, auto H, K M> static constexpr
const V *get(const FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (const V *)hm.values + i;
	}
}


template<class K, class V, size_t C, auto H, K M> static
bool set(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
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

template<class K, class V, size_t C, auto H, K M> static
bool set(FiniteHashmap<K, V, C, H, M> &hm, const K &key, const V &value) noexcept{
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


template<class K, class V, size_t C, auto H, K M> static
bool remove(FiniteHashmap<K, V, C, H, M> &hm, const K &key) noexcept{
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

