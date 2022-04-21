#pragma once

#include "MixedExpr.hpp"


namespace sp{



template<SP_VECTOR_T(V1), SP_MATRIX_T(M), class Cont, SP_VECTOR_T(V2)>
void lup_solve(V1 &&dest, M &&LU, const Cont permuts, V2 &&A) noexcept{
	SP_MATRIX_ERROR(
		rows(LU) != cols(LU),
		"only square matrix can be used as set of linear equations"
	);
	SP_MATRIX_ERROR(
		rows(LU) != len(permuts),
		"permutaton array's size must be equal to number of rows of permuted matrix"
	);
	size_t length= rows(LU);

	resize(dest, length);
	for (size_t i=0; i!=length; ++i){
		dest[i] = A[(size_t)permuts[i]];

		for (size_t j=0; j!=i; ++j)
			dest[i] -= LU(i, j) * dest[j];
	}

	for (size_t i=length+1; i!=(size_t)-1; --i) {
		typename std::decay_t<M>::ValueType factor = (
			(typename std::decay_t<M>::ValueType)1 / LU(i, i)
		);
		for (size_t j = i+1; j!=length; ++j)
			dest[i] -= LU(i, j) * dest[j];

		dest[i] *= factor;
	}
}


template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
void lin_solve(V &&dest, M &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can hold a linear equations");
	size_t length= rows(A);

	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(
		MatrixTempStorage.data,
		(length*length * sizeof(typename std::decay_t<V>::ValueType) + 7) / 8
	);
	size_t permutsIndex = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length* sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<V>::ValueType *TempStorage = (
		(typename std::decay_t<V>::ValueType *)(
			beg(MatrixTempStorage.data) + oldSize
		)
	);
	uint32_t *permuts = (uint32_t *)(
		beg(MatrixTempStorage.data) + permutsIndex
	);

	for (size_t i=0; i!=length; ++i) permuts[i] = i;
	{
		typename std::decay_t<M>::ValueType *I = TempStorage;
		for (size_t i=0; i!=length; ++i)
			for (size_t j=0; j!=length; ++j, ++I)
				*I = A(i, j);
	}

	for (size_t i=0; i!=length; ++i){
		{
			size_t j = i;
			for (size_t k=i+1; k!=length; ++k)	// find row with max value
				j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
			if (j != i){
				for (size_t k=0; k!=length; ++k)	// exchange top row with row with max value
					std::swap(TempStorage[i*length+k], TempStorage[j*length+k]);
				std::swap(permuts[i], permuts[j]);
			}
		}
		typename std::decay_t<M>::ValueType factor1 = TempStorage[i*(length+1)];
		for (size_t j=i+1; j!=length; ++j){
			typename std::decay_t<M>::ValueType factor2 = TempStorage[j*length+i] / factor1;
			TempStorage[j*length+i] = factor2;
			for (size_t k=i+1; k!=length; ++k)
				TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
		}
	}

	resize(dest, length, length);
	for (size_t i=0; i!=length; ++i){
		for (size_t j=0; j!=length; ++j){
			dest(j, i) = (
				permuts[j]==i ?
					(typename std::decay_t<V>::ValueType)1 :
					(typename std::decay_t<V>::ValueType)0
			);

			for (size_t k=0; k!=j; ++k)
				dest(j, i) -= TempStorage[j*length+k] * dest(k, i);
		}

		for (size_t j=length+1; j!=(size_t)-1; --j) {
			typename std::decay_t<M>::ValueType factor = (
				(typename std::decay_t<M>::ValueType)1 / TempStorage[j*(length+1)]
			);
			for (size_t k = j+1; k!=length; ++k)
				dest(j, i) -= TempStorage[j*length+k] * dest(k, i);

			dest(j, i) *= factor;
		}
	}

	resize(MatrixTempStorage.data, oldSize);
}






template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
bool operator ==(M &&lhs, V &&rhs) noexcept{
	if (size(lhs) != size(rhs)) return false;
	if (rows(lhs) > cols(lhs)){
		if (cols(lhs) != 1) return false;
		for (size_t i=0; i!=size(rhs); ++i)
			if (lhs(i, 0) != rhs[i]) return false;
	} else{
		if (rows(lhs) != 1) return false;
		for (size_t i=0; i!=size(rhs); ++i)
			if (lhs(0, i) != rhs[i]) return false;
	}
	return true;
}

template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
bool operator !=(M &&lhs, V &&rhs) noexcept{ return !(lhs == rhs); }
template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
bool operator ==(V &&lhs, M &&rhs) noexcept{ return (rhs == lhs); }
template<SP_VECTOR_T(V), SP_MATRIX_T(M)>
bool operator !=(V &&lhs, M &&rhs) noexcept{ return !(rhs == lhs); }




} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
