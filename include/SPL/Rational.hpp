#pragma once

#include "Utils.hpp"
#include <numeric>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"


template<class Base>
struct Rational{
	Rational(const Rational &) noexcept = default;

	Rational &operator =(const Rational &) noexcept = default;
	
	Rational &operator +=(const Rational rhs) noexcept{
		nom = nom*rhs.den + rhs.nom*den;
		den *= rhs.den;
	}
	
	Rational &operator -=(const Rational rhs) noexcept{
		nom = nom*rhs.den	- rhs.nom*den;
		den *= rhs.den;
	}

	Rational &operator *=(const Rational rhs) noexcept{
		nom *= rhs.nom;
		den *= rhs.den;
	}

	Rational &operator /=(const Rational rhs) noexcept{
		nom *= rhs.den;
		den *= rhs.nom;
	}

	Rational &operator +=(const Base rhs) noexcept{ nom += rhs * den; }
	Rational &operator -=(const Base rhs) noexcept{ nom -= rhs * den; }
	Rational &operator *=(const Base rhs) noexcept{ nom *= rhs; }
	Rational &operator /=(const Base rhs) noexcept{ den *= rhs; }


	operator float() const noexcept{ return (float)nom / (float)den; }
	operator double() const noexcept{ return (double)nom / (double)den; }


	typedef Base ValueType;

	Base nom = Null<Base>;
	Base den = Unit<Base>;
};


template<class B> static
void simplify(Rational<B> &r) noexcept{
	B divider = std::gcd(r.nom, r.den);
	if (divider != 1){
		r.nom /= divider;
		r.den /= divider;
	}
}


template<class B> static constexpr
Rational<B> operator +(Rational <B> lhs, Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den + rhs.nom*lhs.den, lhs.den*rhs.den};
}

template<class B> static constexpr
Rational<B> operator -(Rational <B> lhs, Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den - rhs.nom*lhs.den, lhs.den*rhs.den};
}

template<class B> static constexpr
Rational<B> operator *(Rational <B> lhs, Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.nom, lhs.den*rhs.den};
}

template<class B> static constexpr
Rational<B> operator /(Rational <B> lhs, Rational<B> rhs) noexcept{
	return Rational<B>{lhs.nom*rhs.den, lhs.den*rhs.nom};
}


template<class B> static constexpr
Rational<B> operator +(Rational <B> arg) noexcept{ return arg; }

template<class B> static constexpr
Rational<B> operator -(Rational <B> arg) noexcept{ return Rational<B>{-arg.nom, arg.den}; }



template<class B> static constexpr
bool operator ==(Rational <B> lhs, Rational<B> rhs) noexcept{
	return lhs.nom == rhs.nom && lhs.den == rhs.den;
}

template<class B> static constexpr
bool operator !=(Rational <B> lhs, Rational<B> rhs) noexcept{
	return lhs.nom != rhs.nom || lhs.den != rhs.den;
}

template<class B> static constexpr
bool operator <(Rational <B> lhs, Rational<B> rhs) noexcept{
	typedef std::make_unsigned_t<B> UnsB;
	constexpr size_t half_len = 4 * sizeof(B);
	constexpr B low_mask = UnsB(-1) >> half_len;

	B lhs_lower = (lhs.nom & low_mask) * (rhs.den & low_mask);
	B rhs_lower = (rhs.nom & low_mask) * (lhs.den & low_mask);
	B lhs_upper = (lhs.nom >> half_len) * (rhs.den >> half_len);
	B rhs_upper = (rhs.nom >> half_len) * (lhs.den >> half_len);
	if (signbit(rhs.den) == signbit(lhs.den))
		return (lhs_upper < rhs_upper) || (lhs_upper==rhs_upper && lhs_lower<rhs_lower);
	else
		return (lhs_upper > rhs_upper) || (lhs_upper==rhs_upper && lhs_lower>rhs_lower);
}

template<class B> static constexpr
bool operator <=(Rational <B> lhs, Rational<B> rhs) noexcept{
	typedef std::make_unsigned_t<B> UnsB;
	constexpr size_t half_len = 4 * sizeof(B);
	constexpr B low_mask = UnsB(-1) >> half_len;

	B lhs_upper = (lhs.nom >> half_len) * (rhs.den >> half_len);
	B rhs_upper = (rhs.nom >> half_len) * (lhs.den >> half_len);
	B lhs_lower = (lhs.nom & low_mask) * (rhs.den & low_mask);
	B rhs_lower = (rhs.nom & low_mask) * (lhs.den & low_mask);
	return (lhs_upper <= rhs_upper) || (lhs_upper==rhs_upper && (UnsB)lhs_lower<=(UnsB)rhs_lower);
}

template<class B> static constexpr
bool operator >(Rational <B> lhs, Rational<B> rhs) noexcept{ return rhs < lhs; }

template<class B> static constexpr
bool operator >=(Rational <B> lhs, Rational<B> rhs) noexcept{ return rhs <= lhs; }



template<class B> static constexpr 
Rational<B> operator +(Rational<B> lhs, B rhs) noexcept{
	return Rational<B>{lhs.nom + rhs*lhs.den, lhs.den};
}

template<class B> static constexpr 
Rational<B> operator -(Rational<B> lhs, B rhs) noexcept{
	return Rational<B>{lhs.nom - rhs*lhs.den, lhs.den};
}

template<class B> static constexpr 
Rational<B> operator *(Rational<B> lhs, B rhs) noexcept{
	return Rational<B>{lhs.nom * rhs, lhs.den};
}

template<class B> static constexpr 
Rational<B> operator /(Rational<B> lhs, B rhs) noexcept{
	return Rational<B>{lhs.nom, lhs.den * rhs};
}


template<class B> static constexpr 
Rational<B> operator +(B lhs, Rational<B> rhs) noexcept{
	return Rational<B>{rhs.nom + lhs*rhs.den, rhs.den};
}

template<class B> static constexpr 
Rational<B> operator -(B lhs, Rational<B> rhs) noexcept{
	return Rational<B>{rhs.nom - lhs*rhs.den, rhs.den};
}

template<class B> static constexpr 
Rational<B> operator *(B lhs, Rational<B> rhs) noexcept{
	return Rational<B>{rhs.nom * lhs, rhs.den};
}

template<class B> static constexpr 
Rational<B> operator /(B lhs, Rational<B> rhs) noexcept{
	return Rational<B>{lhs * rhs.den, rhs.nom};
}


#pragma GCC diagnostic pop

