#pragma once

#include "SPL/Allocators.hpp"
#include "SPL/Arrays.hpp"
#include "SPL/Complex.hpp"

#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

DynamicArray<uint64_t, MallocAllocator{}> PolynomialTempStorage;


// POLYNOMIAL EXPRESSION CLASSES
template<class Base> struct PolynomialExpr : Base{
	typedef typename std::decay_t<Base>::ValueType ValueType;

	template<class T>
	T operator ()(T x) const noexcept{
		T result = (T)(*this)[0];
		T x_power = Unit<T>;
		for (size_t i=1; i<len(*this); ++i){
			x_power *= x;
			result += (*this)[i] * x_power;
		}
		return result;
	}
};



template<class Arg1, class Arg2>
struct PolynomialExprAdd{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::ValueType ValueType;
	constexpr static bool HasWrongDegree = true;

	constexpr ValueType operator [](size_t index) const noexcept{
		return arg1[index] + arg2[index];
	}
};
template<class Arg1, class Arg2> constexpr
size_t len(const PolynomialExprAdd<Arg1, Arg2> &e) noexcept{
	return max(len(e.arg1), len(e.arg2));
}

template<class Arg1, class Arg2>
struct PolynomialExprSubtract{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::ValueType ValueType;
	constexpr static bool HasWrongDegree = true;

	constexpr ValueType operator [](size_t index) const noexcept{
		return arg1[index] - arg2[index];
	}
};
template<class Arg1, class Arg2> constexpr
size_t len(const PolynomialExprSubtract<Arg1, Arg2> &e) noexcept{
	return max(len(e.arg1), len(e.arg2));
}

template<class Arg1, class Arg2>
struct PolynomialExprMultiply{
	Arg1 arg1;
	Arg2 arg2;

	typedef typename std::decay_t<Arg1>::ValueType ValueType;
	constexpr static bool HasWrongDegree = (
		std::decay_t<Arg1>::HasWrongDegree || std::decay_t<Arg2>::HasWrongDegree
	);

	constexpr ValueType operator [](size_t index) const noexcept{
		ValueType result = ValueType{0};

		const ssize_t skipSize = index+1 - len(arg2);
		size_t i = skipSize<0 ? 0 : skipSize;
		size_t j = min(index, len(arg2)-1);
		for (; i!=len(arg1) && j!=(size_t)-1; ++i, --j)
			result += arg1[i] * arg2[j];

		return result;
	}
};
template<class Arg1, class Arg2> constexpr
size_t len(const PolynomialExprMultiply<Arg1, Arg2> &e) noexcept{
	return len(e.arg1) + len(e.arg2) - 1;
}

template<class Arg>
struct PolynomialExprDifferentiate{
	Arg arg;

	typedef typename std::decay_t<Arg>::ValueType ValueType;
	constexpr static bool HasWrongDegree = Arg::HasWrongDegree;

	constexpr ValueType operator [](size_t index) const noexcept{
		return (ValueType{index+1} * arg[index+1]);
	}
};
template<class Arg> constexpr
size_t len(const PolynomialExprDifferentiate<Arg> &e) noexcept{
	return len(e.arg) - 1;
}

template<class Arg>
struct PolynomialExprDifferentiate2{
	Arg arg;

	typedef typename std::decay_t<Arg>::ValueType ValueType;
	constexpr static bool HasWrongDegree = Arg::HasWrongDegree;

	constexpr ValueType operator [](size_t index) const noexcept{
		const auto temp = (ValueType)(index + 1);
		return temp*(temp+1) * arg[index+2];
	}
};
template<class Arg> constexpr
size_t len(const PolynomialExprDifferentiate2<Arg> &e) noexcept{
	return len(e.arg) - 2;
}

template<class Arg>
struct PolynomialExprIntegrate{
	Arg arg;

	typedef typename std::decay_t<Arg>::ValueType ValueType;
	constexpr static bool HasWrongDegree = Arg::HasWrongDegree;

	constexpr ValueType operator [](size_t index) const noexcept{
		if (!index) return (ValueType)0;
		return arg[index-1] / (ValueType)index;
	}
};
template<class Arg> constexpr
size_t len(const PolynomialExprIntegrate<Arg> &e) noexcept{
	return len(e.arg) + 1;
}

template<class Arg>
struct PolynomialExprScalarMultiply{
	Arg arg;
	typename std::decay_t<Arg>::ValueType scalar;

	typedef typename std::decay_t<Arg>::ValueType ValueType;
	constexpr static bool HasWrongDegree = false;

	constexpr ValueType operator [](size_t index) const noexcept{ return arg[index]*scalar; }
};
template<class Arg> constexpr
size_t len(const PolynomialExprScalarMultiply<Arg> &e) noexcept{
	return e.scalar==0 ? (size_t)1 : len(e.arg);
}



// POLYNOMIAL CONTAINER
template<class Base = DynamicArray<double, MallocAllocator{}>> struct Polynomial;

template<class Base>
void repair_degree(Polynomial<Base> &p) noexcept{
	size_t degr = len(p);
	do{ --degr; } while (p[degr]==(typename Base::ValueType)0 && degr!=0);
	resize(p, degr + 1);
}


template<class Base>
struct Polynomial : Base{
	typedef typename Base::ValueType ValueType;
	static constexpr bool HasWrongDegree = false;

	template<class B>
	const Polynomial &operator =(const Polynomial<B> &rhs) noexcept{
		resize(*this, len(rhs));
		auto J = beg(*this);
		for (auto I=beg(rhs); I!=end(rhs); ++I, ++J) *J = *I;
		return *this;
	}

	template<class B>
	const Polynomial &operator =(const PolynomialExpr<B> &rhs) noexcept{
		resize(*this, len(rhs));
		for (size_t i=0; i!=len(*this); ++i) (*this)[i] = rhs[i];
		if (B::HasWrongDegree) repair_degree(*this);
		return *this;
	}


	template<class B>
	const Polynomial &operator +=(const Polynomial<B> &rhs) noexcept{
		resize(*this, max(len(*this), len(rhs)));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] += rhs[i];
		repair_degree(*this);
		return *this;
	}

	template<class B>
	const Polynomial &operator -=(const Polynomial<B> &rhs) noexcept{
		resize(this, max(len(*this), len(rhs)));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] -= rhs[i];
		repair_degree(*this);
		return *this;
	}
	
	template<class T>
	const Polynomial &operator *=(Polynomial::ValueType rhs) noexcept{
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] *= rhs;
		return *this;
	}
	
	template<class B>
	const Polynomial &operator /=(Polynomial::ValueType rhs) noexcept{
		Polynomial::ValueType divider = Unit<ValueType> / rhs;
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] *= divider;
		return *this;
	}

	template<class B>
	const Polynomial &operator *=(const Polynomial<B> &rhs) noexcept{
		size_t old_size = len(*this);
		size_t prev_storage_size = len(PolynomialTempStorage);
		expand_back(PolynomialTempStorage, old_size + ((sizeof(ValueType)+7) / 8));
		ValueType *temp_storage = (ValueType *)(
			beg(PolynomialTempStorage) + prev_storage_size
		);
		for (size_t i=0; i!=len(*this); i+=1) temp_storage[i] = (*this)[i];
		resize(*this, old_size + len(rhs) - 1);

		for (size_t i=0; i!=old_size; ++i)
			for (size_t j=0; j!=len(rhs); ++j)
				(*this)[i+j] += temp_storage[i] * rhs[j];
		resize(PolynomialTempStorage, prev_storage_size);
		return *this;
	}

	template<class B>
	const Polynomial &operator +=(const PolynomialExpr<B> &rhs) noexcept{
		resize(*this, max(len(*this), len(rhs)));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] += rhs[i];
		repair_degree(*this);
		return *this;
	}

	template<class B>
	const Polynomial &operator -=(const PolynomialExpr<B> &rhs) noexcept{
		resize(*this, max(len(*this), len(rhs)));
		for (size_t i=0; i!=len(*this); ++i)
			(*this)[i] -= rhs[i];
		repair_degree(*this);
		return *this;
	}

	template<class B>
	const Polynomial &operator *=(const PolynomialExpr<B> &rhs) noexcept{
		size_t old_size = len(*this);
		size_t prev_storage_size = len(PolynomialTempStorage);
		expand_back(PolynomialTempStorage, old_size + ((sizeof(ValueType)+7) / 8));
		ValueType *temp_storage = (ValueType *)(
			beg(PolynomialTempStorage) + prev_storage_size
		);
		for (size_t i=0; i!=len(*this); i+=1) temp_storage[i] = (*this)[i];
		resize(*this, old_size + len(rhs) - 1);

		for (size_t i=0; i!=old_size; ++i)
			for (size_t j=0; j!=len(rhs); ++j)
				(*this)[i+j] += temp_storage[i] * rhs[j];
		if (B::HasWrongDegree) repair_degree(*this); 
		resize(PolynomialTempStorage, prev_storage_size);
		return *this;
	}

	template<class T>
	T operator ()(T x) const noexcept{
		T result = (T)(*this)[0];
		T x_power = Unit<T>;
		for (size_t i=1; i<len(*this); ++i){
			x_power *= x;
			result += (*this)[i] * x_power;
		}
		return result;
	}
};



// POLYNOMIAL OPERATIONS
template<class Base> constexpr
size_t degree(const Polynomial<Base> &p) noexcept{ return len(p) - 1; }

template<class Base> constexpr
size_t degree(const PolynomialExpr<Base> &p) noexcept{ return len(p) - 1; }


template<class B1, class B2>
auto operator +(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const Polynomial<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<const Polynomial<B1> &, const Polynomial<B2> &>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const PolynomialExpr<B1> &lhs, const Polynomial<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<B1>, const Polynomial<B2> &>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const Polynomial<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<const Polynomial<B1> &, PolynomialExpr<B2>>>{{lhs, rhs}};
}

template<class B1, class B2>
auto operator +(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprAdd<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator -(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprSubtract<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}
template<class B1, class B2>
auto operator *(const PolynomialExpr<B1> &lhs, const PolynomialExpr<B2> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprMultiply<PolynomialExpr<B1>, PolynomialExpr<B2>>>{{lhs, rhs}};
}



template<class B>
auto diff(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate<const Polynomial<B> &>>{{arg}};
}
template<class B>
auto diff2(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate2<const Polynomial<B> &>>{{arg}};
}
template<class B>
auto integrate(const Polynomial<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprIntegrate<const Polynomial<B> &>>{{arg}};
}


template<class B>
auto diff(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate<PolynomialExpr<B>>>{{arg}};
}
template<class B>
auto diff2(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprDifferentiate2<PolynomialExpr<B>>>{{arg}};
}
template<class B>
auto integrate(const PolynomialExpr<B> &arg) noexcept{
	return PolynomialExpr<PolynomialExprIntegrate<PolynomialExpr<B>>>{{arg}};
}


template<class B, class T>
auto operator *(const Polynomial<B> &lhs, T rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<const Polynomial<B> &>>{{lhs, rhs}};
}
template<class B, class T>
auto operator *(T lhs, const Polynomial<B> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<const Polynomial<B> &>>{{rhs, lhs}};
}
template<class B, class T>
auto operator *(const PolynomialExpr<B> &lhs, T rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<PolynomialExpr<B>>>{{lhs, rhs}};
}
template<class B, class T>
auto operator *(T lhs, const PolynomialExpr<B> &rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<PolynomialExpr<B>>>{{rhs, lhs}};
}


template<class B, class T>
auto operator /(const Polynomial<B> &lhs, T rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<const Polynomial<B> &>>{{lhs, Unit<T>/rhs}};
}
template<class B, class T>
auto operator /(const PolynomialExpr<B> &lhs, T rhs) noexcept{
	return PolynomialExpr<PolynomialExprScalarMultiply<PolynomialExpr<B>>>{{lhs, Unit<T>/rhs}};
}


template<class Cont, class B2, class B3>
void divide(
	Cont &result, Polynomial<B2> &arg1, const Polynomial<B3> &arg2
) noexcept{
	typedef typename B2::ValueType ValueType;

	size_t last_arg2_index = len(arg2) - 1;
	size_t last_result_index = len(arg1) - len(arg2);
	if (last_result_index > SIZE_MAX/2){
		resize(result, 1);
		result[0] = ValueType{};
		return;
	}
	resize(result, last_result_index + 1);

	ValueType divider = Unit<ValueType> / arg2[len(arg2)-1];
	for (size_t i=last_result_index; i!=(size_t)-1; --i){
		ValueType res = arg1[len(arg1)-1] * divider;
		pop(arg1);
		for (size_t j=last_arg2_index; j!=(size_t)-1; --j)
			arg1[i+j] -= res * arg2[j];
		result[i] = res;
	}
}

template<class Cont, class B2, class B3>
void divide(
	Cont &result, Polynomial<B2> &arg1, const PolynomialExpr<B3> &arg2
) noexcept{
	typedef typename B2::ValueType ValueType;

	size_t last_arg2_index = len(arg2) - 1;
	size_t last_result_index = len(arg1) - len(arg2);
	if (last_result_index > SIZE_MAX/2){
		resize(result, 1);
		result[0] = ValueType{};
		return;
	}
	resize(result, last_result_index + 1);

	ValueType divider = Unit<ValueType> / arg2[len(arg2)-1];
	for (size_t i=last_result_index; i!=(size_t)-1; --i){
		ValueType res = arg1[len(arg1)-1] * divider;
		pop(arg1);
		for (size_t j=last_arg2_index; j!=(size_t)-1; --j)
			arg1[i+j] -= res * arg2[j];
		result[i] = res;
	}
}



template<class Cont, class Base, size_t max_iterations = 100>
bool find_roots(
	Cont &results, const Polynomial<Base> &arg, typename Cont::ValueType precission
) noexcept{
	typedef typename Cont::ValueType ValueType;

	resize(results, 0);
	size_t required_size = ((len(arg) * sizeof(ValueType) + 7) / 8)*2;
	size_t prev_storage_size = len(PolynomialTempStorage);
	expand_back(PolynomialTempStorage, required_size);
	
	Polynomial<Span<ValueType>> pol1{{
		(ValueType *)(beg(PolynomialTempStorage) + prev_storage_size),
		len(arg)
	}};
	Polynomial<Span<ValueType>> pol2{{end(pol1), len(arg)}};
	PolynomialExpr<Polynomial<FiniteArray<ValueType, 2>>> divider;
	resize(divider, 2);

	pol1 = arg;
	for (size_t i=0; i<degree(arg); ++i){
		ValueType curr_x = pol1[len(pol1)-1] / pol1[0];
		ValueType prev_x;

		size_t iter_counter = 0;
		do{
			if (iter_counter == max_iterations){
				resize(PolynomialTempStorage, prev_storage_size);
				return true;
			} ++iter_counter;
			ValueType pol_x = pol1(curr_x);
			if (pol_x == ValueType{}) break;
			prev_x = curr_x;
			curr_x -= pol1(curr_x) / diff(pol1)(curr_x);
		} while (precission < abs(curr_x-prev_x));
		push_value(results, curr_x);

		divider[0] = -curr_x;
		divider[1] = (ValueType)1;

		divide(pol2, pol1, divider);
		swap(pol1, pol2);
	}
	resize(PolynomialTempStorage, prev_storage_size);
	return false;
}

template<class Cont, class Base, size_t max_iterations = 100>
bool find_roots(
	Cont &results, const PolynomialExpr<Base> &arg, typename Cont::ValueType precission
) noexcept{
	typedef typename Base::ValueType ValueType;

	resize(results, 0);
	size_t required_size = ((len(arg) * sizeof(ValueType) + 7) / 8)*2;
	size_t prev_storage_size = len(PolynomialTempStorage);
	expand_back(PolynomialTempStorage, required_size);
	
	Polynomial<Span<ValueType>> pol1{{
		(ValueType *)(beg(PolynomialTempStorage) + prev_storage_size),
		len(arg)
	}};
	Polynomial<Span<ValueType>> pol2{{end(pol1), len(arg)}};
	PolynomialExpr<Polynomial<FiniteArray<ValueType, 2>>> divider;
	resize(divider, 2);

	pol1 = arg;
	for (size_t i=0; i<degree(arg); ++i){
		ValueType curr_x = pol1[len(pol1)-1] / pol1[0];
		ValueType prev_x;

		size_t iter_counter = 0;
		do{
			if (iter_counter == max_iterations){
				resize(PolynomialTempStorage, prev_storage_size);
				return true;
			} ++iter_counter;
			ValueType pol_x = pol1(curr_x);
			if (pol_x == ValueType{}) break;
			prev_x = curr_x;
			curr_x -= pol1(curr_x) / diff(pol1)(curr_x);
		} while (precission < abs(curr_x-prev_x));
		push_value(results, curr_x);

		divider[0] = -curr_x;
		divider[1] = (ValueType)1;

		divide(pol2, pol1, divider);
		swap(pol1, pol2);
	}
	resize(PolynomialTempStorage, prev_storage_size);
	return false;
}


template<class Cont, class Base, size_t max_iterations = 100>
bool find_croots(
	Cont &results, const Polynomial<Base> &arg, typename Cont::ValueType::ValueType precission
) noexcept{
	typedef typename Cont::ValueType ComplexType;
	typedef typename ComplexType::ValueType RealType;
	size_t len = degree(arg);

	resize(results, len);
	{
		float radius = (float)abs(
			(RealType)len * arg[0] / (arg[1] + arg[1])
		) + (float)abs(
			arg[len-1] / ((RealType)(2*len) * arg[len])
		);
		float angle = (2.f*(float)M_PI) / (float)len;

		float offset = angle * 0.25f;
		for (auto I=beg(results); I!=end(results); ++I){
			I->real = (RealType)(radius * cosf(offset));
			I->imag = (RealType)(radius * sinf(offset));
			offset += angle;
		}
	}


	size_t iter_counter = 0;
	ComplexType prev_root;
	do{
		if (iter_counter == max_iterations) return true;
		++iter_counter;
		prev_root = results[0];
		for (size_t j=0; j!=len; ++j){
			ComplexType curr_root = results[j];
			ComplexType arg_z = arg(curr_root);

			ComplexType sum = Null<ComplexType>;
			for (size_t k=0; k!=j; ++k)
				sum += Unit<ComplexType> / (curr_root - results[k]);
			for (size_t k=j+1; k!=len; ++k)
				sum += Unit<ComplexType> / (curr_root - results[k]);

			results[j] -= arg_z / (diff(arg)(curr_root) - arg_z * sum);
		}
	} while (precission < abs(results[0] - prev_root));
	return false;
}

template<class Cont, class Base, size_t max_iterations = 100>
bool find_croots(
	Cont &results, const PolynomialExpr<Base> &arg, typename Cont::ValueType::ValueType precission
) noexcept{
	typedef typename Cont::ValueType ComplexType;
	typedef typename ComplexType::ValueType RealType;
	size_t len = degree(arg);

	resize(results, len);
	{
		float radius = (float)abs(
			(RealType)len * arg[0] / (arg[1] + arg[1])
		) + (float)abs(
			arg[len-1] / ((RealType)(2*len) * arg[len])
		);
		float angle = (2.f*(float)M_PI) / (float)len;

		float offset = angle * 0.25f;
		for (auto I=beg(results); I!=end(results); ++I){
			I->real = (RealType)(radius * cosf(offset));
			I->imag = (RealType)(radius * sinf(offset));
			offset += angle;
		}
	}


	size_t iter_counter = 0;
	ComplexType prev_root;
	do{
		if (iter_counter == max_iterations) return true;
		++iter_counter;
		prev_root = results[0];
		for (size_t j=0; j!=len; ++j){
			ComplexType curr_root = results[j];
			ComplexType arg_z = arg(curr_root);

			ComplexType sum = Null<ComplexType>;
			for (size_t k=0; k!=j; ++k)
				sum += Unit<ComplexType> / (curr_root - results[k]);
			for (size_t k=j+1; k!=len; ++k)
				sum += Unit<ComplexType> / (curr_root - results[k]);

			results[j] -= arg_z / (diff(arg)(curr_root) - arg_z * sum);
		}
	} while (precission < abs(results[0] - prev_root));
	return false;
}



/*
template<class Cont, class Base, size_t max_iterations = 100>
bool find_roots(
	Cont &results, const Polynomial<Base> &arg, typename Base::ValueType precission
) noexcept{
	typedef typename Base::ValueType RealType;
	size_t len = degree(arg);

	resize(results, len);
	{
		float radius = (float)abs(
			RealType{len} * arg[0] / (arg[1] + arg[1])
		) + (float)abs(
			arg[len-1] / (RealType{2*len} * arg[len])
		);
		float angle = (2.0*M_PI) / (float)len;

		float offset = angle * 0.25;
		for (auto I=beg(results); I!=end(results); ++I){
			*I = RealType{radius * cosf(offset)};
			offset += angle;
		}
	}


	size_t iter_counter = 0;
	RealType prev_root;
	do{
		if (iter_counter == max_iterations) return true;
		++iter_counter;
		prev_root = results[0];
		for (size_t j=0; j!=len; ++j){
			RealType curr_root = results[j];
			RealType arg_z = arg(curr_root);

			RealType sum = RealType{0};
			for (size_t k=0; k!=j; ++k)
				sum += RealType{1} / (curr_root - results[k]);
			for (size_t k=j+1; k!=len; ++k)
				sum += RealType{1} / (curr_root - results[k]);

			results[j] -= arg_z / (diff(arg)(curr_root) - arg_z * sum);
		}
	} while (precission < abs(results[0] - prev_root));
	return false;
}
*/

#pragma GCC diagnostic pop
