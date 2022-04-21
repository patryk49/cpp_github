#pragma once

#include "Utils.hpp"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

namespace sp{

template<class Base>
struct Rational{
	constexpr Rational(const Rational &) noexcept = default;
	constexpr Rational &operator =(const Rational &) noexcept = default;
	
	constexpr Rational &operator +=(const Rational rhs) noexcept{
		nom = nom*rhs.den + rhs.nom*den;
		den *= rhs.den;
	}
	constexpr Rational &operator -=(const Rational rhs) noexcept{
		nom = nom*rhs.den	- rhs.nom*den;
		den *= rhs.den;
	}
	constexpr Rational &operator *=(const Rational rhs) noexcept{
		nom *= rhs.nom;
		den *= rhs.den;
	}
	constexpr Rational &operator /=(const Rational rhs) noexcept{
		nom *= rhs.den;
		den *= rhs.nom;
	}

	constexpr Rational &operator +=(const Base rhs) noexcept{ nom += rhs * den; }
	constexpr Rational &operator -=(const Base rhs) noexcept{ nom -= rhs * den; }
	constexpr Rational &operator *=(const Base rhs) noexcept{ nom *= rhs; }
	constexpr Rational &operator /=(const Base rhs) noexcept{ den *= rhs; }

	constexpr void simplify() noexcept{
		const Base divider = sp::gcd(nom, den);
		if (divider != 1){
			nom /= divider;
			den /= divider;
		}
	}

	operator float() const noexcept{ return (float)nom / (float)den; }
	operator double() const noexcept{ return (double)nom / (double)den; }


	typedef Base ValueType;

	Base nom = sp::Null<Base>;
	Base den = sp::Unit<Base>;
};



template<class B>
constexpr Rational<B> operator +(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den + rhs.nom*lhs.den, lhs.den*rhs.den};
}

template<class B>
constexpr Rational<B> operator -(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den - rhs.nom*lhs.den, lhs.den*rhs.den};
}

template<class B>
constexpr Rational<B> operator *(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.nom, lhs.den*rhs.den};
}

template<class B>
constexpr Rational<B> operator /(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den, lhs.den*rhs.nom};
}


template<class B>
constexpr Rational<B> operator +(const Rational <B> arg) noexcept{ return arg; }

template<class B>
constexpr Rational<B> operator -(const Rational <B> arg) noexcept{ return Rational<B>{-arg.nom, arg.den}; }


template<class B>
constexpr bool operator ==(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return lhs.nom == rhs.nom && lhs.den == rhs.den;
}

template<class B>
constexpr bool operator !=(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	return lhs.nom != rhs.nom || lhs.den != rhs.den;
}

template<class B>
constexpr bool operator <(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	typedef std::make_unsigned_t<B> UnsB;
	constexpr size_t halfLen = 4 * sizeof(B);
	constexpr B lowMask = UnsB(-1) >> halfLen;

	B lhsLower = (lhs.nom & lowMask) * (rhs.den & lowMask);
	B rhsLower = (rhs.nom & lowMask) * (lhs.den & lowMask);
	const B lhsUpper = (lhs.nom >> halfLen) * (rhs.den >> halfLen);
	const B rhsUpper = (rhs.nom >> halfLen) * (lhs.den >> halfLen);
	if (signbit(rhs.den) == signbit(lhs.den))
		return (lhsUpper < rhsUpper) || (lhsUpper==rhsUpper && lhsLower<rhsLower);
	else
		return (lhsUpper > rhsUpper) || (lhsUpper==rhsUpper && lhsLower>rhsLower);
}

template<class B>
constexpr bool operator <=(const Rational <B> lhs, const Rational<B> rhs) noexcept{
	typedef std::make_unsigned_t<B> UnsB;
	constexpr size_t halfLen = 4 * sizeof(B);
	constexpr B lowMask = UnsB(-1) >> halfLen;

	const B lhsUpper = (lhs.nom >> halfLen) * (rhs.den >> halfLen);
	const B rhsUpper = (rhs.nom >> halfLen) * (lhs.den >> halfLen);
	const B lhsLower = (lhs.nom & lowMask) * (rhs.den & lowMask);
	const B rhsLower = (rhs.nom & lowMask) * (lhs.den & lowMask);
	return (lhsUpper <= rhsUpper) || (lhsUpper==rhsUpper && (UnsB)lhsLower<=(UnsB)rhsLower);
}

template<class B>
constexpr bool operator >(const Rational <B> lhs, const Rational<B> rhs) noexcept{ return rhs < lhs; }

template<class B>
constexpr bool operator >=(const Rational <B> lhs, const Rational<B> rhs) noexcept{ return rhs <= lhs; }




template<class B>
constexpr Rational<B> operator +(const Rational <B> lhs, const B rhs) noexcept{
	return Rational<B>{lhs.nom + rhs*lhs.den, lhs.den};
}

template<class B>
constexpr Rational<B> operator -(const Rational <B> lhs, const B rhs) noexcept{
	return Rational<B>{lhs.nom - rhs*lhs.den, lhs.den};
}

template<class B>
constexpr Rational<B> operator *(const Rational <B> lhs, const B rhs) noexcept{
	return Rational<B>{lhs.nom * rhs, lhs.den};
}

template<class B>
constexpr Rational<B> operator /(const Rational <B> lhs, const B rhs) noexcept{
	return Rational<B>{lhs.nom, lhs.den * rhs};
}


template<class B>
constexpr Rational<B> operator +(const B lhs, const Rational <B> rhs) noexcept{
	return Rational<B>{rhs.nom + lhs*rhs.den, rhs.den};
}

template<class B>
constexpr Rational<B> operator -(const B lhs, const Rational <B> rhs) noexcept{
	return Rational<B>{rhs.nom - lhs*rhs.den, rhs.den};
}

template<class B>
constexpr Rational<B> operator *(const B lhs, const Rational <B> rhs) noexcept{
	return Rational<B>{rhs.nom * lhs, rhs.den};
}

template<class B>
constexpr Rational<B> operator /(const B lhs, const Rational <B> rhs) noexcept{
	return Rational<B>{lhs * rhs.den, rhs.nom};
}



#pragma GCC diagnostic pop

} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
