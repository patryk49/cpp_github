#pragma once

#define SP_CI constexpr inline
#define SP_SI static inline
#define SP_CSI constexpr static inline

#include "Initializators.hpp"

#undef SP_CI
#undef SP_SI
#undef SP_CSI

namespace sp{


template<class T, bool RowMajor = true>
using Matrix = MatrixWrapper<MatrixDynamic<T, RowMajor, sp::MallocAllocator<>>>;

template<class T, size_t R, size_t C, bool RowMajor = true>
using FixedMatrix = MatrixWrapper<MatrixFixed<T, RowMajor, R, C>>;

template<class T, size_t C, bool RowMajor = true>
using FiniteMatrix = MatrixWrapper<MatrixFinite<T, RowMajor, C>>;

template<class T, class A = sp::MallocAllocator<>, bool RowMajor = true>
using DynamicMatrix = MatrixWrapper<MatrixDynamic<T, RowMajor, A>>;




template<class T>
using Vector = VectorWrapper<VectorDynamic<T, sp::MallocAllocator<>>>;

template<class T, size_t L>
using FixedVector = VectorWrapper<VectorFixed<T, L>>;

template<class T, size_t C>
using FiniteVector = VectorWrapper<VectorFinite<T, C>>;

template<class T, class A = sp::MallocAllocator<>>
using DynamicVector = VectorWrapper<VectorDynamic<T, A>>;












} // END OF NAMESPACE
