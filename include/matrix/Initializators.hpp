#pragma once
#include "matrix/MixedOperations.hpp"


namespace sp{
using namespace priv__;

template<SP_MATRIX_T(M), SP_VECTOR_T(V)>
void init_vandermonde(M &&dest, V &&A, const size_t degree) noexcept{
	typedef typename std::decay_t<M>::ValueType T;
	dest.resize(A.size(), degree+1);
	if constexpr (std::decay_t<M>::RowMajor){
		for (size_t i=0; i!=dest.rows(); ++i){
			dest(i, 0) = (T)1;
			T xVal = A[i];
			T accVal = xVal;
			for (size_t j=1; j!=dest.cols(); ++j){
				dest(i, j) = accVal;
				accVal *= xVal;
			}
		}
	} else{
		for (size_t i=0; i!=dest.rows(); ++i)
			dest(i, 0) = (T)1;
		if (degree)
			for (size_t i=0; i!=dest.rows(); ++i)
				dest(i, 1) = A[i];
		for (size_t i=2; i!=dest.cols(); ++i)
			for (size_t j=0; j!=dest.rows(); ++j)
				dest(j, i) = dest(j, i-1) * A[j];
	}
}


} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////////////////////////
