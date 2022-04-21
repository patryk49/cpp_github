#pragma once

#include "SPL/Polynomial.hpp"


namespace sp{


template<class NomBase = sp::Polynomial<>, class DenBase = NomBase>
struct Ratiomial{
	NomBase nom;
	DenBase den;
	
	typedef typename NomBase::ValueType NomValueType;
	typedef typename NomBase::ValueType DenValueType;
	typedef decltype(NomValueType{} / DenValueType{}) ValueType;

	template<class NomB, class DenB>
	const Ratiomial &operator =(const Ratiomial<NomB, DenB> &rhs) noexcept{
		this->nom = rhs.nom;
		this->den = rhs.den;
		return *this;
	}
	
	template<class NomB, class DenB>
	const Ratiomial &operator +=(const Ratiomial<NomB, DenB> &rhs) noexcept{
		this->nom *= rhs.den;
		this->den *= rhs.den;
		this->nom += rhs.nom * this->den;
		return *this;
	}
	
	template<class NomB, class DenB>
	const Ratiomial &operator -=(const Ratiomial<NomB, DenB> &rhs) noexcept{
		this->nom *= rhs.den;
		this->den *= rhs.den;
		this->nom -= rhs.nom * this->den;
		return *this;
	}
	
	template<class NomB, class DenB>
	const Ratiomial &operator *=(const Ratiomial<NomB, DenB> &rhs) noexcept{
		this->nom *= rhs.nom;
		this->den *= rhs.den;
		return *this;
	}
	
	template<class NomB, class DenB>
	const Ratiomial &operator /=(const Ratiomial<NomB, DenB> &rhs) noexcept{
		this->nom *= rhs.den;
		this->den *= rhs.nom;
		return *this;
	}

	auto operator ()(const NomValueType x) const noexcept{
		NomValueType nomResult = this->nom[0];
		{
			NomValueType xPower = (NomValueType)1;
			for (size_t i=1; i<std::size(this->nom); ++i){
				xPower *= x;
				nomResult += this->nom[i] * xPower;
			}
		}
		DenValueType denResult = this->den[0];
		{
			DenValueType xPower = (DenValueType)1;
			for (size_t i=1; i<std::size(this->den); ++i){
				xPower *= x;
				denResult += this->den[i] * xPower;
			}
		}
		return nomResult / denResult;
	}
};


template<class NB, class DB>
size_t degree(const Ratiomial<NB, DB> &p) noexcept{ return sp::degree(p.nom) - sp::degree(p.den); }


template<class N1, class D1, class N2, class D2>
auto operator *(const Ratiomial<N1, D1> &lhs, const Ratiomial<N2, D2> &rhs) noexcept{
	return Ratiomial<
		PolynomialExpr<PolynomialExprMultiply<N1, N2>>,
		PolynomialExpr<PolynomialExprMultiply<D1, D2>>
	>{{{{lhs.nom}, {rhs.nom}}}, {{{lhs.den}, {rhs.den}}}};
}

template<class N1, class D1, class N2, class D2>
auto operator /(const Ratiomial<N1, D1> &lhs, const Ratiomial<N2, D2> &rhs) noexcept{
	return Ratiomial<
		PolynomialExpr<PolynomialExprMultiply<N1, D2>>,
		PolynomialExpr<PolynomialExprMultiply<D1, N2>>
	>{{{{lhs.nom}, {rhs.den}}}, {{{lhs.den}, {rhs.nom}}}};
}

template<class N1, class D1, class N2, class D2>
auto operator +(const Ratiomial<N1, D1> &lhs, const Ratiomial<N2, D2> &rhs) noexcept{
	return Ratiomial<
		PolynomialExpr<PolynomialExprAdd<
			PolynomialExpr<PolynomialExprMultiply<N1, D2>>,
			PolynomialExpr<PolynomialExprMultiply<N2, D1>>
		>>,
		PolynomialExpr<PolynomialExprMultiply<D1, D2>>
	>{{{{{{lhs.nom}, {rhs.den}}}, {{{rhs.nom}, {lhs.den}}}}}, {{{lhs.den}, {rhs.den}}}};
}

template<class N1, class D1, class N2, class D2>
auto operator -(const Ratiomial<N1, D1> &lhs, const Ratiomial<N2, D2> &rhs) noexcept{
	return Ratiomial<
		PolynomialExpr<PolynomialExprSubtract<
			PolynomialExpr<PolynomialExprMultiply<N1, D2>>,
			PolynomialExpr<PolynomialExprMultiply<N2, D1>>
		>>,
		PolynomialExpr<PolynomialExprMultiply<D1, D2>>
	>{{{{{{lhs.nom}, {rhs.den}}}, {{{rhs.nom}, {lhs.den}}}}}, {{{lhs.den}, {rhs.den}}}};
}

template<class N, class D>
auto diff(const Ratiomial<N, D> &arg) noexcept{
	return Ratiomial<
		PolynomialExpr<PolynomialExprSubtract<
			PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<PolynomialExprDifferentiate<N>>, D>>,
			PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<PolynomialExprDifferentiate<D>>, N>>
		>>,
		PolynomialExpr<PolynomialExprMultiply<D, D>>
	>{{{{{{arg.nom}, {arg.den}}}, {{{arg.den}, {arg.nom}}}}}, {{{arg.den}, {arg.den}}}};
}



} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
