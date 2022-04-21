#pragma once

#include "Utils.hpp"
#include <math.h>

namespace sp{

template<class T = double>
struct Complex{
	T real, imag;
	typedef T ValueType;

	constexpr Complex<T> &operator +=(Complex<T> rhs) noexcept{
		this->real += rhs.real;
		this->imag += rhs.imag;
		return *this;
	}
	constexpr Complex<T> &operator -=(Complex<T> rhs) noexcept{
		this->real -= rhs.real;
		this->imag -= rhs.imag;
		return *this;
	}
	constexpr Complex<T> &operator *=(Complex<T> rhs) noexcept{
		T realHolder = this->real;
		this->real = realHolder*rhs.real - this->imag*rhs.imag;
		this->imag = realHolder*rhs.imag + this->imag*rhs.real;
		return *this;
	}	
	constexpr Complex<T> &operator /=(Complex<T> rhs) noexcept{
		T divider = rhs.real*rhs.real + rhs.imag*rhs.imag;
		T realHolder = this->real;
		this->real = (realHolder*rhs.real + this->imag*rhs.imag) / divider;
		this->imag = (this->imag*rhs.real - realHolder*rhs.imag) / divider;
		return *this;
	}

	constexpr Complex<T> &operator +=(T rhs) noexcept{
		this->real += rhs;
		return *this;
	}
	constexpr Complex<T> &operator -=(T rhs) noexcept{
		this->real -= rhs;
		return *this;
	}
	constexpr Complex<T> &operator *=(T rhs) noexcept{
		this->real *= rhs;
		this->imag *= rhs;
		return *this;
	}	
	constexpr Complex<T> &operator /=(T rhs) noexcept{
		this->real /= rhs;
		this->imag /= rhs;
		return *this;
	}

	template<class TT>
	constexpr explicit operator Complex<TT>() const noexcept{
		return Complex<TT>{(TT)real, (TT)imag};
	}
	constexpr explicit operator T() const noexcept{ return real; }
};





template<class TT> 
constexpr auto operator +(Complex<TT> lhs, Complex<TT> rhs) noexcept{
	return Complex<TT>{lhs.real + rhs.real, lhs.imag + rhs.imag};
}
template<class TT>
constexpr auto operator -(Complex<TT> lhs, Complex<TT> rhs) noexcept{
	return Complex<TT>{lhs.real - rhs.real, lhs.imag - rhs.imag};
}
template<class TT>
constexpr auto operator *(Complex<TT> lhs, Complex<TT> rhs) noexcept{ 
	return Complex<TT>{
		lhs.real*rhs.real - lhs.imag*rhs.imag,
		lhs.real*rhs.imag + lhs.imag*rhs.real
	};
}
template<class TT>
constexpr auto operator /(Complex<TT> lhs, Complex<TT> rhs) noexcept{
	TT divider = (TT)1 / (rhs.real*rhs.real + rhs.imag*rhs.imag);
	return Complex<TT>{
		(lhs.real*rhs.real + lhs.imag*rhs.imag) * divider,
		(lhs.imag*rhs.real - lhs.real*rhs.imag) * divider
	};
}

template<class TT>
constexpr auto operator +(Complex<TT> arg) noexcept{ return arg; }

template<class TT>
constexpr auto operator -(Complex<TT> arg) noexcept{
	return Complex<TT>{-arg.real, -arg.imag};
}


template<class TT>
constexpr auto operator +(Complex<TT> lhs, TT rhs) noexcept{
	return Complex<TT>{lhs.real + rhs, lhs.imag};
}
template<class TT>
constexpr auto operator -(Complex<TT> lhs, TT rhs) noexcept{
	return Complex<TT>{lhs.real - rhs, lhs.imag};
}
template<class TT>
constexpr auto operator *(Complex<TT> lhs, TT rhs) noexcept{
	return Complex<TT>{lhs.real * rhs, lhs.imag * rhs};
}
template<class TT>
constexpr auto operator /(Complex<TT> lhs, TT rhs) noexcept{
	return Complex<TT>{lhs.real / rhs, lhs.imag / rhs};
}

template<class TT>
constexpr auto operator +(TT lhs, Complex<TT> rhs) noexcept{
	return Complex<TT>{lhs + rhs.real, rhs.imag};
}
template<class TT>
constexpr auto operator -(TT lhs, Complex<TT> rhs) noexcept{
	return Complex<TT>{lhs - rhs.real, rhs.imag};
}
template<class TT>
constexpr auto operator *(TT lhs, Complex<TT> rhs) noexcept{
	return Complex<TT>{lhs * rhs.real, lhs * rhs.imag};
}
template<class TT>
constexpr auto operator /(TT lhs, Complex<TT> rhs) noexcept{
	TT multiplyer = lhs / (lhs.real*rhs.real + lhs.imag*rhs.imag);
	return Complex<TT>{rhs.real * multiplyer, -rhs.imag * multiplyer};
}


template<class TT>
SP_CSI TT abs(Complex<TT> arg) noexcept{
	return sqrt(arg.real*arg.real + arg.imag*arg.imag);
}

template<class TT>
constexpr TT absSquare(Complex<TT> arg) noexcept{
	return arg.real*arg.real + arg.imag*arg.imag;
}

template<class TT>
constexpr TT arg(Complex<TT> arg) noexcept{
	return std::atan2(arg.imag, arg.real);
}
template<class TT>
constexpr TT real(Complex<TT> arg) noexcept{
	return arg.real;
}
template<class TT>
constexpr TT imag(Complex<TT> arg) noexcept{
	return arg.imag;
}

template<class TT>
constexpr Complex<TT> conj(Complex<TT> arg) noexcept{
	return Complex<TT>{arg.real, -arg.imag};
}
	
	
template<class TT>
constexpr Complex<TT> exp(Complex<TT> arg) noexcept{
	TT modul = ::std::exp(arg.real);
	return Complex<TT>{modul*::std::cos(arg.imag), modul*::std::sin(arg.imag)};
}
template<class TT>
constexpr Complex<TT> log(Complex<TT> arg) noexcept{
	return Complex<TT>{::log(absSquare(arg)) / (TT)2, ::std::atan2(arg.imag, arg.real)};
}
template<class TT>
constexpr Complex<TT> pow(Complex<TT> base, Complex<TT> exponent) noexcept{
	return exp(::std::log(base) * exponent);
}
template<class TT>
constexpr Complex<TT> pow(TT base, Complex<TT> exponent) noexcept{
	TT modul = ::std::pow(base, exponent.real);
	TT angle = ::std::log(base) * exponent.imag;
	return Complex<TT>{modul*::std::cos(angle), modul*::std::sin(angle)};
}
template<class TT>
constexpr Complex<TT> pow(Complex<TT> base, TT exponent) noexcept{
	TT modul = ::std::pow(absSquare(base), exponent / (TT)2);
	TT angle = ::std::atan2(base.imag, base.real) * exponent;
	return Complex<TT>{modul*::std::cos(angle), modul*::std::sin(angle)};
}

template<class TT>
constexpr Complex<TT> sin(Complex<TT> arg) noexcept{
	return Complex<TT>{::std::sin(arg.real)*::std::cosh(arg.imag), ::std::cos(arg.real)*::std::sinh(arg.imag)};
}
template<class TT>
constexpr Complex<TT> cos(Complex<TT> arg) noexcept{
	return Complex<TT>{::std::cos(arg.real)*::std::cosh(arg.imag), -::std::sin(arg.real)*::std::sinh(arg.imag)};
}
template<class TT>
constexpr Complex<TT> tan(Complex<TT> arg) noexcept{
	TT rCos = ::std::cos(arg.real);
	TT iSinh = ::std::sinh(arg.imag);
	TT denom = rCos*rCos + iSinh*iSinh;
	return Complex<TT>{::std::sin(arg.real)*rCos/denom, iSinh*::std::cosh(arg.imag)/denom};
}



} // END OF NAMESPACE ///////////////////////////////////////
