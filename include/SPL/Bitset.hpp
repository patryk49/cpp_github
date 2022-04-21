#pragma once

#include "Utils.hpp"


namespace sp{ // BEGINING OF NAMESPACE ////////////////////////////////////////////////////////////



template<size_t C>
struct FiniteBitset{
	constexpr static size_t UnitBits = sizeof(size_t) * 8;
	constexpr static size_t UnitCount = (C + UnitBits-1) / UnitBits;

	constexpr static size_t Mask = (1 << UnitBits) - 1;

	constexpr static size_t Capacity = UnitBits * UnitCount;
	static_assert(C != 0, "this makes no sense");

	constexpr bool operator [](size_t index) const noexcept{
		return (this->data[index >> UnitBits] >> (index & Mask)) & 1;
	}

	union{
		size_t data[UnitCount];
	};
	size_t size = 0;
};

template<size_t C>
SP_CSI void set(FiniteBitset<C> &bits, size_t index) noexcept{
	bits.data[index >> decltype(bits)::UnitBits] |= 1 << (index & decltype(bits)::Mask);
}

template<size_t C>
SP_CSI void unset(FiniteBitset<C> &bits, size_t index) noexcept{
	bits.data[index >> decltype(bits)::UnitBits] &= ~(1 << (index & decltype(bits)::Mask));
}

template<size_t C>
SP_CSI void flip(FiniteBitset<C> &bits, size_t index) noexcept{
	bits.data[index >> decltype(bits)::UnitBits] ^= 1 << (index & decltype(bits)::Mask);
}

template<size_t C>
SP_CSI void set(FiniteBitset<C> &bits, size_t index, bool x) noexcept{
	bits.data[index >> decltype(bits)::UnitBits] &= ~(1 << (index & decltype(bits)::Mask));
	bits.data[index >> decltype(bits)::UnitBits] |= x << (index & decltype(bits)::Mask);
}

template<size_t C>
SP_CSI size_t len(const FiniteBitset<C> &bits) noexcept{ return bits.size; }

template<size_t C>
SP_CSI size_t cap(const FiniteBitset<C> &bits) noexcept{ return decltype(bits)::Capacity; }

template<size_t C>
SP_CSI bool front(const FiniteBitset<C> &bits) noexcept{ return bits.data[0] & 1; }

template<size_t C>
SP_CSI bool back(const FiniteBitset<C> &bits) noexcept{
	return (
		bits.data[
			(bits.size-1) >> decltype(bits)::UnitBits
		] >> ((bits.size-1) & decltype(bits)::Mask)
	) & 1;
}

template<size_t C>
SP_CSI bool is_full(const FiniteBitset<C> &bits) noexcept{
	return bits.size == decltype(bits)::Capacity;
}

template<size_t C>
SP_CSI bool is_empty(const FiniteBitset<C> &bits) noexcept{ return bits.size == 0; }


template<size_t C>
SP_CSI void push(FiniteBitset<C> &bits, bool arg) noexcept{
	bits.data[bits.size >> decltype(bits)::UnitBits] &= ~(1 << (bits.size & decltype(bits)::Mask));
	bits.data[bits.size >> decltype(bits)::UnitBits] |= arg << (bits.size & decltype(bits)::Mask);
	++bits.size;
}

template<size_t C> SP_CSI void pop(FiniteBitset<C> &bits) noexcept{ --bits.size; }

template<size_t C>
SP_CSI bool pop_val(FiniteBitset<C> &bits, bool arg) noexcept{
	--bits.size;
	return (
		bits.data[bits.size >> decltype(bits)::UnitBits] >> (bits.size & decltype(bits)::Mask)
	) & 1;
}

template<size_t C>
SP_CSI void resize(FiniteBitset<C> &bits, size_t size) noexcept{
//	bits.data[
//		bits.size >> decltype(bits)::UnitBits
//	] &= ~((size_t)-1 >> (bits.size & decltype(bits)::Mask));
	
	bits.size = size;
}

template<size_t C>
SP_CSI void shrink_back(FiniteBitset<C> &bits, size_t amount) noexcept{
	resize(bits, bits.size-amount);
}

template<size_t C>
SP_CSI void expand_back(FiniteBitset<C> &bits, size_t amount) noexcept{
	resize(bits, bits.size+amount);
}

template<size_t C0, size_t C1>
bool operator ==(const FiniteBitset<C0> &lhs, const FiniteBitset<C1> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	size_t units = lhs.size >> decltype(lhs)::UnitBits;
	size_t i = 0;
	for (; i!=units; ++i) if (lhs.data[i] != rhs.data[i]) return false;
	if ((lhs[i] ^ rhs[i]) >> (lhs.size & decltype(lhs)::Mask)) return false;
	return true;
}

template<size_t C0, size_t C1>
bool operator !=(const FiniteBitset<C0> &lhs, const FiniteBitset<C1> &rhs) noexcept{
	return !(lhs == rhs);
}




} // END OF NAMESPACE	///////////////////////////////////////////////////////////////////////////
