#pragma once

#include "SPL/Utils.hpp"
#include "SPL/Allocators.hpp"
#include "SPL/Utils.hpp"


namespace Expr{

enum class Type : int16_t{
	Solid, Variable,
	
	Minus,

	Add, Subtract, Multiply, Divide, Power,
	Sqrt, Cbrt,
};

union Node{
	Type type;
};

template<Type T, class C>
struct Expr : C{
	constexpr static Type Type;
}


// OPERATOR / FUNCTION EXPRESSIONS
template<class A>
struct ExprSolid : Expr<Type::Solid>{
	A nodes;
	
	template<Type T>
	ExprSolid &operator =(const Expr<T> &rhs) noexcept{
		switch (T){
		case Type::Add:
			resize(this->nodes, );
			copy(this->nodes, rhs.rhs)


		}
	}
};

template<class E>
struct ExprUnary{
	E arg;
};

template<class EL, class ER>
struct ExprBinary{
	EL lhs;
	ER rhs;
};

template<class EL, class EM, class ER>
struct ExprTernary{
	EL lhs;
	EM mhs;
	ER rhs;
};

template<Type T, class CL, class CR>
void push_expr(Expr<Type::Solid, CL> dest, const Expr<T, CR> &expr){
	switch (T){
	case Type::Minus:
		push_expr(dest, expr.rhs);
		push_value(dest.nodes, Node{T});
		break;
	case Type::Add:
	case Type::Subtract:
	case Type::Multiply:
	case Type::Divide:
	case Type::Power:
		push_expr(dest, expr.rhs);
		push_expr(dest, expr.lhs);
		push_value(dest.nodes, Node{T});
		break;



	}

}


// OPEARATORS
template<class T> using ProperType = std::conditional_t<EL::Type==Type::Solid, const T &, T>;


template<class EL, class ER>
auto operator +(const Expr<EL> &lhs, const Expr<ER> &rhs) noexcept{
	return ExprBin<Type::Add, ProperType<EL>, ProperType<ER>>{rhs, lhs};
}

template<class EL, class ER>
auto operator -(const Expr<EL> &lhs, const Expr<ER> &rhs) noexcept{
	return ExprBin<Type::Subtract, ProperType<EL>, ProperType<ER>>{rhs, lhs};
}

template<class EL, class ER>
auto operator *(const Expr<EL> &lhs, const Expr<ER> &rhs) noexcept{
	return ExprBin<Type::Multiply, ProperType<EL>, ProperType<ER>>{rhs, lhs};
}

template<class EL, class ER>
auto operator /(const Expr<EL> &lhs, const Expr<ER> &rhs) noexcept{
	return ExprBin<Type::Divide, ProperType<EL>, ProperType<ER>>{rhs, lhs};
}

template<class EL, class ER>
auto operator ^(const Expr<EL> &lhs, const Expr<ER> &rhs) noexcept{
	return ExprBin<Type::Power, ProperType<EL>, ProperType<ER>>{rhs, lhs};
}






















} // END OF NAMESPACE /////////////////////////////////////////////////////////////////////////////
