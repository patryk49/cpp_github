#include "Utils.hpp"

template<size_t B = 16, class T = int>
struct FixedPoint{
	typedef T Base;
	static constexpr size_t FracBits = B;


	static_assert(
		std::is_integral_v<T>,
		"Fixed point must be built on top of iteger."
	);
	static_assert(
		FracBits < sizeof(T)*8,
		"Number of fractional bits must be smaller than total number of bits."
	);

	template<size_t BR, class TR>
	auto &operator +=(FixedPoint<BR, TR> rhs){
		this->data += ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

	template<size_t BR, class TR>
	auto &operator -=(FixedPoint<BR, TR> rhs){
		this->data -= ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

	template<uint32_t BR, class TR>
	auto &operator *=(FixedPoint<BR, TR> rhs){
		typedef IntOfGivenSize<std::bit_ceil(sizeof(T)+sizeof(TR))> MaxType;
		this->data = (T)((MaxType)this->data * (MaxType)rhs.data) >> BR;
		return *this;
	}

	template<uint32_t BR, class TR>
	auto &operator /=(FixedPoint<BR, TR> rhs){
		typedef IntOfGivenSize<std::bit_ceil(sizeof(T) + BR)> MaxType;
		this->data = (T)(((MaxType)this->data << BR) / (MaxType)rhs.data);
		return *this;
	}

	template<size_t BR, class TR>
	auto &operator %=(FixedPoint<BR, TR> rhs){
		this->data %= ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

	template<size_t BR, class TR>
	auto &operator |=(FixedPoint<BR, TR> rhs){
		this->data |= ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

	template<size_t BR, class TR>
	auto &operator &=(FixedPoint<BR, TR> rhs){
		this->data &= ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

	template<size_t BR, class TR>
	auto &operator ^=(FixedPoint<BR, TR> rhs){
		this->data ^= ((FixedPoint<B, T>)rhs).data;
		return *this;
	}

// ASSIGN
	template<size_t BR, class TR>
	const auto &operator =(FixedPoint<BR, TR> rhs) noexcept{
		constexpr ssize_t points_distance = BR - B;
		if constexpr (points_distance >= 0)
			this->data = rhs.data >> points_distance;
		else
			this->data = rhs.data << -points_distance;
		return *this;
	}

// CONVERSIONS
	template<size_t BR, class TR> constexpr
	operator FixedPoint() const noexcept{
		constexpr ssize_t points_distance = B - BR;
		if constexpr (points_distance >= 0)
			return FixedPoint<BR, TR>{this->data >> points_distance};
		else
			return FixedPoint<BR, TR>{this->data << -points_distance};
	}
	
	constexpr
	operator T() const noexcept { return (T)(this->data >> B); }
	
	constexpr explicit
	operator float() const noexcept{
		return (float)this->data / (float)(1 << B);
	}
	
	constexpr explicit
	operator double() const noexcept{
		return (double)this->data / (double)(1 << B);
	}
	
	constexpr explicit
	operator long double() const noexcept{
		return (long double)this->data / (long double)(1 << B);
	}

	T data;
};




template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator +(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data + ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator -(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data - ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator *(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	typedef IntOfGivenSize<std::bit_ceil(sizeof(TL)+sizeof(TR))> MaxType;
	return FixedPoint<BL, TL>{
		(TL)(((MaxType)lhs.data * (MaxType)rhs.data) >> BR)
	};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator /(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	typedef IntOfGivenSize<std::bit_ceil((sizeof(TL)*8 + BR + 7) / 8)> MaxType;
	return FixedPoint<BL, TL>{
		(TL)(((MaxType)lhs.data << BR) / (MaxType)rhs.data)
	};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator %(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data % ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator |(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data | ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator &(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data & ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t BL, class TL, size_t BR, class TR> static constexpr
auto operator ^(FixedPoint<BL, TL> lhs, FixedPoint<BR, TR> rhs){
	return FixedPoint<BL, TL>{lhs.data ^ ((FixedPoint<BL, TL>)rhs).data};
}

template<size_t B, class T> static constexpr
auto operator ~(FixedPoint<B, T> x){ return FixedPoint<B, T>{~x.data}; }

