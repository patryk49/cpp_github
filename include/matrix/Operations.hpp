#pragma once

#include "Expr.hpp"


namespace sp{

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void transpose(M1 &&dest, M2 &&A) noexcept{
	resize(dest, A.cols(), A.rows());
	size_t blockSize = CacheBlockLen / sizeof(A(0, 0));
	
	for (size_t i=0; i<rows(dest); i+=blockSize)
		for (size_t j=0; j<cols(dest); j+=blockSize)
			for (size_t ii=i; ii<i+blockSize; ++ii)
				for (size_t jj=j; jj<j+blockSize; ++jj)
					dest(ii, jj) = A(jj, ii);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(M3)>
void kron_product(M1 &&dest, M2 &&A, M3 &&B) noexcept{
	resize(dest, A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			typename std::decay_t<M2>::ValueType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = tempVal * B(k, l);
		}
}

template<auto operation, SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(B3)>
void kron_apply(M1 &&dest, M2 &&A, B3 &&B) noexcept{
	resize(dest, A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			typename std::decay_t<M2>::ValueType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = operation(tempVal, B(k, l));
		}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2), SP_MATRIX_T(M3), class Operation>
void kron_apply(M1 &&dest, M2 &&A, M3 &&B, Operation &&operation) noexcept{
	resize(dest, A.rows()*B.rows(), A.cols()*B.cols());
	
	for (size_t i=0; i!=rows(A); ++i)
		for (size_t j=0; j!=cols(A); ++j){
			typename std::decay_t<M2>::ValueType tempVal = A(i, j);
			for (size_t k=0; k!=rows(B); ++k)
				for (size_t l=0; l!=cols(B); ++l)
					dest(i*rows(B)+k, j*cols(B)+l) = operation(tempVal, B(k, l));
		}
}


template<SP_MATRIX_T(M)>
void swap_rows(M &&dest, size_t row1, size_t row2) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i)
		swap(dest(row1, i), dest(row2, i));
}

template<SP_MATRIX_T(M)>
void swap_cols(M &&dest, size_t col1, size_t col2) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i)
		swap(dest(i, col1), dest(i, col2));
}

template<SP_MATRIX_T(M)>
void scale_row(M &&dest, size_t row, typename ::std::decay_t<M>::ValueType s) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i) dest(row, i) *= s;
}

template<SP_MATRIX_T(M)>
void scale_col(M &&dest, size_t col, typename ::std::decay_t<M>::ValueType s) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i) dest(i, col) *= s;
}

template<SP_MATRIX_T(M)>
void add_rows(M &&dest, size_t rowDest, size_t rowSrc,
	typename ::std::decay_t<M>::ValueType s = (typename ::std::decay_t<M>::ValueType)1
) noexcept{
	for (size_t i=0; i!=dest.cols(); ++i) dest(rowDest, i) += s * dest(rowSrc, i);
}

template<SP_MATRIX_T(M)>
void add_cols(M &&dest, size_t colDest, size_t colSrc,
	typename ::std::decay_t<M>::ValueType s = (typename ::std::decay_t<M>::ValueType)1
) noexcept{
	for (size_t i=0; i!=dest.rows(); ++i) dest(i, colDest) += s * dest(i, colSrc);
}


template<SP_MATRIX_T(M)>
void lu_decompose(M &&dest) noexcept{
	size_t length = min(rows(dest), cols(dest));

	typename std::decay_t<M>::ValueType factor1, factor2;
	if constexpr (std::decay_t<M>::RowMajor){
		for (size_t i=0; i!=length; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(j, i)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						swap(dest(i, k), dest(j, k));
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(j, i) / factor1;
				dest(j, i) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(j, k) -= dest(i, k) * factor2;
			}
		}
	} else{
		for (size_t i=0; i!=length; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(i, j)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						swap(dest(k, i), dest(k, j));
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(i, j) / factor1;
				dest(i, j) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(k, j) -= dest(k, j) * factor2;
			}
		}
	}
}

template<SP_MATRIX_T(M), class Cont>
void lup_decompose(M &&dest, Cont &permuts) noexcept{
	resize(permuts, rows(dest));
	for (size_t i=0; i!=rows(dest); ++i) permuts[i] = i;

	size_t length = min(rows(dest), cols(dest));

	typename std::decay_t<M>::ValueType factor1, factor2;
	if constexpr (std::decay_t<M>::RowMajor){
		for (size_t i=0; i!=length; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(j, i)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						swap(dest(i, k), dest(j, k));
					swap(permuts[i], permuts[j]);
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(j, i) / factor1;
				dest(j, i) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(j, k) -= dest(i, k) * factor2;
			}
		}
	} else{
		for (size_t i=0; i!=length; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=rows(dest); ++k)	// find row with max value
					j = abs(dest(k, i))>abs(dest(i, j)) ? k : j;
				if (j != i){
					for (size_t k=0; k!=cols(dest); ++k)	// exchange top row with row with max value
						swap(dest(k, i), dest(k, j));
					swap(permuts[i], permuts[j]);
				}
			}
			factor1 = dest(i, i);
			for (size_t j=i+1; j!=rows(dest); ++j){
				factor2 = dest(i, j) / factor1;
				dest(i, j) = factor2;
				for (size_t k=i+1; k!=cols(dest); ++k)
					dest(k, i) -= dest(k, j) * factor2;
			}
		}
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void extract_lower(M1 &&dest, M2 &&src) noexcept{
	size_t length = min(rows(src), cols(src));

	resize(dest, rows(src), length);
	resize(src, length, cols(src));

	for (size_t i=0; i!=length; ++i){
		for (size_t j=i+1; j!=rows(dest); ++j){
			dest(j, i) = src(j, i);
			src(j, i) = (typename std::decay_t<M1>::ValueType)0;
		}
		dest(i, i) = (typename std::decay_t<M1>::ValueType)1;
		for (size_t j=i+1; j!=cols(src); ++j)
			dest(i, j) = (typename std::decay_t<M1>::ValueType)0;
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void extract_upper(M1 &&dest, M2 &&src) noexcept{
	size_t length = min(rows(src), cols(src));
	
	resize(dest, length, cols(src));
	resize(src, rows(src), length);

	for (size_t i=0; i!=length; ++i){
		for (size_t j=i+1; j!=cols(dest); ++j){
			dest(i, j) = src(i, j);
			src(i, j) = (typename std::decay_t<M1>::ValueType)0;
		}
		dest(i, i) = (typename std::decay_t<M1>::ValueType)1;
		for (size_t j=i+1; j!=rows(src); ++j)
			dest(j, i) = (typename std::decay_t<M1>::ValueType)0;
	}
}

template<SP_MATRIX_T(M)>
void cholesky_decompose(M &&dest) noexcept{
	SP_MATRIX_ERROR(rows(dest) != cols(dest), "only square matrix can be cholesky decomposed");
	size_t length = rows(dest);
	if (std::decay_t<M>::RowMajor){
		for (size_t i=0; i!=length; ++i){
			for (size_t j=0; j!=i; ++j){
				typename std::decay_t<M>::ValueType sum = (typename std::decay_t<M>::ValueType)0;
				for (size_t k=0; k!=j; ++k)
					sum += dest(i, k) * dest(j, k);

				dest(i, j) = (dest(i, j) - sum) / dest(j, j);
			}
			{
				typename std::decay_t<M>::ValueType sum = (typename std::decay_t<M>::ValueType)0;
				for (size_t k=0; k!=i; ++k)
					sum += dest(i, k) * dest(i, k);
				
				dest(i, i) = sqrt(dest(i, i) - sum);
			}
		}
	} else{
		for (size_t i=0; i!=length; ++i){
			for (size_t j=0; j!=i; ++j){
				typename std::decay_t<M>::ValueType sum = (typename std::decay_t<M>::ValueType)0;
				for (size_t k=0; k!=j; ++k)
					sum += dest(k, i) * dest(k, j);

				dest(j, i) = (dest(j, i) - sum) / dest(j, j);
			}
			{
				typename std::decay_t<M>::ValueType sum = (typename std::decay_t<M>::ValueType)0;
				for (size_t k=0; k!=i; ++k)
					sum += dest(k, i) * dest(k, i);
				
				dest(i, i) = sqrt(dest(i, i) - sum);
			}
		}
	}
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void cholesky_update(M1 &&dest, M2 &&A) noexcept{
	SP_MATRIX_ERROR(rows(dest)!=size(A) || cols(dest)!=size(A),
		"non square matrices cannot be cholesky decomposed"
	);
	size_t length = rows(dest);
	for (size_t i=0; i!=length; ++i){
		auto r = sqrt(dest(i, i)*dest(i, i) + A[i]*A[i]);
		auto c = r / dest(i, i);
		auto s = A[i] / dest(i, i);
		dest(i, i) = r;
		for (size_t j=i+1; j!=length; ++j){
			dest(j, i) = (dest(j, i) + s * A[j]) / c;
			A[j] = c * A[j] - s * dest(j, i);
		}
	}
}


template<SP_MATRIX_T(M), class Cont>
void permute_rows(M &&dest, const Cont &permuts) noexcept{
	static_assert(std::is_integral_v<typename Cont::ValueType>,
		"permutation array must contain integral values"
	);
	SP_MATRIX_ERROR(rows(dest) != len(permuts),
		"row count of permuted matrix must be the same as size of permutation array"
	);

	size_t length = len(permuts);
	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length * sizeof(typename Cont::ValueType) + 7) / 8);
	typename Cont::ValueType *TempStorage = (typename Cont::ValueType *)beg(MatrixTempStorage.data) + oldSize;
	
	for (size_t i=0; i!=length; ++i) TempStorage[i] = beg(permuts)[i];
	
	for (size_t i=0; i!=length; ++i)
		while (TempStorage[i] != i){
			size_t swapIndex = TempStorage[i];
			iswap(TempStorage+i, TempStorage+swapIndex);

			for (size_t j=0; j!=cols(dest); ++j)
				swap(dest(swapIndex, j), dest(i, j));
		}

	resize(MatrixTempStorage.data, oldSize);	
}

template<SP_MATRIX_T(M), class Cont>
void permute_cols(M &&dest, const Cont &permuts) noexcept{
	static_assert(std::is_integral_v<typename Cont::ValueType>, "permutation array must contain integral values");
	SP_MATRIX_ERROR(cols(dest) != len(permuts), "columns count of permuted matrix must be the same as size of permutation array");

	size_t length = len(permuts);
	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length * sizeof(typename Cont::ValueType) + 7) / 8);
	typename Cont::ValueType *TempStorage = (typename Cont::ValueType *)beg(MatrixTempStorage.data) + oldSize;
	
	for (size_t i=0; i!=length; ++i) TempStorage[i] = beg(permuts)[i];
	
	for (size_t i=0; i!=length; ++i)
		while (TempStorage[i] != i){
			size_t swapIndex = TempStorage[i];
			iswap(TempStorage+i, TempStorage+swapIndex);

			for (size_t j=0; j!=cols(dest); ++j)
				swap(dest(j, swapIndex), dest(j, i));
		}

	resize(MatrixTempStorage.data, oldSize);	
}

template<SP_MATRIX_T(M)>
void invert(M &&dest) noexcept{
	SP_MATRIX_ERROR(rows(dest) != cols(dest), "only square matrix can be inverted");
	size_t length = rows(dest);

	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length*length * sizeof(typename std::decay_t<M>::ValueType) + 7) / 8);
	size_t permutsIndex = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length * sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<M>::ValueType *TempStorage = (typename std::decay_t<M>::ValueType *)(beg(MatrixTempStorage.data) + oldSize);
	uint32_t *permuts = (uint32_t *)(beg(MatrixTempStorage.data) + permutsIndex);

	for (size_t i=0; i!=length; ++i) permuts[i] = i;

	if constexpr (std::decay_t<M>::RowMajor){
		typename std::decay_t<M>::ValueType *I = TempStorage;
		for (size_t i=0; i!=length; ++i)
			for (size_t j=0; j!=length; ++j, ++I)
				*I = dest(j, i);
	} else{
		typename std::decay_t<M>::ValueType *I = TempStorage;
		for (size_t i=0; i!=length; ++i)
			for (size_t j=0; j!=length; ++j, ++I)
				*I = dest(i, j);
	}

	typename std::decay_t<M>::ValueType factor1, factor2;
	for (size_t i=0; i!=length; ++i){
		{
			size_t j = i;
			for (size_t k=i+1; k!=length; ++k)	// find row with max value
				j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
			if (j != i){
				for (size_t k=0; k!=length; ++k)	// exchange top row with row with max value
					swap(TempStorage[i*length+k], TempStorage[j*length+k]);
				swap(permuts[i], permuts[j]);
			}
		}
		factor1 = TempStorage[i*(length+1)];
		for (size_t j=i+1; j!=length; ++j){
			factor2 = TempStorage[j*length+i] / factor1;
			TempStorage[j*length+i] = factor2;
			for (size_t k=i+1; k!=length; ++k)
				TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
		}
	}

	if constexpr (std::decay_t<M>::	RowMajor){
		for (size_t i=0; i!=length; ++i){
			for (size_t j=0; j!=length; ++j){
				dest(i, j) = permuts[j]==i ? (typename std::decay_t<M>::ValueType)1 : (typename std::decay_t<M>::ValueType)0;

				for (size_t k=0; k!=j; ++k)
					dest(i, j) -= TempStorage[j*length+k] * dest(i, k);
			}

			for (size_t j=length+1; j!=(size_t)-1; --j) {
				for (size_t k = j+1; k!=length; ++k)
					dest(i, j) -= TempStorage[j*length+k] * dest(i, k);

				dest(i, j) /= TempStorage[j*(length+1)];
			}
		}
	} else{
		for (size_t i=0; i!=length; ++i){
			for (size_t j=0; j!=length; ++j){
				dest(j, i) = permuts[j]==i ? (typename std::decay_t<M>::ValueType)1 : (typename std::decay_t<M>::ValueType)0;

				for (size_t k=0; k!=j; ++k)
					dest(j, i) -= TempStorage[j*length+k] * dest(k, i);
			}

			for (size_t j=length+1; j!=(size_t)-1; --j) {
				for (size_t k = j+1; k!=length; ++k)
					dest(j, i) -= TempStorage[j*length+k] * dest(k, i);

				dest(j, i) /= TempStorage[j*(length+1)];
			}
		}
	}

	resize(MatrixTempStorage.data, oldSize);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void invert(M1 &&dest, M2 &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can be inverted");
	size_t length= rows(A);

	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length*length * sizeof(typename std::decay_t<M1>::ValueType) + 7) / 8);
	size_t permutsIndex = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length* sizeof(uint32_t) + 7) / 8);
	typename std::decay_t<M1>::ValueType *TempStorage = (typename std::decay_t<M1>::ValueType *)(beg(MatrixTempStorage.data) + oldSize);
	uint32_t *permuts = (uint32_t *)(beg(MatrixTempStorage.data) + permutsIndex);

	for (size_t i=0; i!=length; ++i) permuts[i] = i;
	{
		typename std::decay_t<M2>::ValueType *I = TempStorage;
		for (size_t i=0; i!=length; ++i)
			for (size_t j=0; j!=length; ++j, ++I)
				*I = A(i, j);
	}


	// L = F*msqrt(D)
	// inv(A) = inv(tr(L)) * inv(L) = inv(tr(F*msqrt(D))) * inv(F*msqrt(D)) = tr(inv(F)) * tr(inv(msqrt(D))) * inv(msqrt(D)) * inv(F)

	for (size_t i=0; i!=length; ++i){
		{
			size_t j = i;
			for (size_t k=i+1; k!=length; ++k)	// find row with max value
				j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
			if (j != i){
				for (size_t k=0; k!=length; ++k)	// exchange top row with row with max value
					swap(TempStorage[i*length+k], TempStorage[j*length+k]);
				swap(permuts[i], permuts[j]);
			}
		}
		typename std::decay_t<M2>::ValueType factor1 = TempStorage[i*(length+1)];
		for (size_t j=i+1; j!=length; ++j){
			typename std::decay_t<M2>::ValueType factor2 = TempStorage[j*length+i] / factor1;
			TempStorage[j*length+i] = factor2;
			for (size_t k=i+1; k!=length; ++k)
				TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
		}
	}

	resize(dest, length, length);
	for (size_t i=0; i!=length; ++i){
		for (size_t j=0; j!=length; ++j){
			dest(j, i) = permuts[j]==i ? (typename std::decay_t<M1>::ValueType)1 : (typename std::decay_t<M1>::ValueType)0;

			for (size_t k=0; k!=j; ++k)
				dest(j, i) -= TempStorage[j*length+k] * dest(k, i);
		}

		for (size_t j=length+1; j!=(size_t)-1; --j) {
			typename std::decay_t<M2>::ValueType factor = (typename std::decay_t<M2>::ValueType)1 / TempStorage[j*(length+1)];
			for (size_t k = j+1; k!=length; ++k)
				dest(j, i) -= TempStorage[j*length+k] * dest(k, i);

			dest(j, i) *= factor;
		}
	}
	resize(MatrixTempStorage.data, oldSize);
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
void pinvert(M1 &&dest, M2 &&A) noexcept{
	if (rows(A) > cols(A)){
		size_t length= cols(A);

		size_t oldSize = len(MatrixTempStorage.data);
		expand_back(MatrixTempStorage.data, (length*length * sizeof(typename std::decay_t<M1>::ValueType) + 7) / 8);
		size_t permutsIndex = len(MatrixTempStorage.data);
		expand_back(MatrixTempStorage.data, (length* sizeof(uint32_t) + 7) / 8);
		typename std::decay_t<M1>::ValueType *TempStorage = (typename std::decay_t<M1>::ValueType *)(beg(MatrixTempStorage.data) + oldSize);
		uint32_t *permuts = (uint32_t *)(beg(MatrixTempStorage.data) + permutsIndex);

		for (size_t i=0; i!=length; ++i) permuts[i] = i;
		if constexpr (std::decay_t<M2>::RowMajor){
			typename std::decay_t<M2>::ValueType *I = TempStorage;
			size_t innLen = rows(A);
			for (size_t i=0; i!=length; ++i)
				for (size_t j=0; j!=i+1; ++j, ++I){
					typename std::decay_t<M2>::ValueType sum{};
					for (size_t k=0; k!=innLen; ++k)
						sum += A(i, k) * A(j, k);
					*I = sum;
				}
		}

		// typename std::decay_t<M>::ValueType factor1, factor2;
		// for (size_t i=0; i!=length; ++i){
		// 	{
		// 		size_t j = i;
		// 		for (size_t k=i+1; k!=length; ++k)	// find row with max value
		// 			j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
		// 		if (j != i){
		// 			for (size_t k=0; k!=length; ++k)	// exchange top row with row with max value
		// 				swap(TempStorage[i*length+k], TempStorage[j*length+k]);
		// 			swap(permuts[i], permuts[j]);
		// 		}
		// 	}
		// 	factor1 = TempStorage[i*(length+1)];
		// 	for (size_t j=i+1; j!=length; ++j){
		// 		factor2 = TempStorage[j*length+i] / factor1;
		// 		TempStorage[j*length+i] = factor2;
		// 		for (size_t k=i+1; k!=length; ++k)
		// 			TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
		// 	}
		// }

		// if constexpr (std::decay_t<M>::RowMajor){
		// 	for (size_t i=0; i!=length; ++i){
		// 		for (size_t j=0; j!=length; ++j){
		// 			dest(i, j) = permuts[j]==i ? (typename std::decay_t<M>::ValueType)1 : (typename std::decay_t<M>::ValueType)0;

		// 			for (size_t k=0; k!=j; ++k)
		// 				dest(i, j) -= TempStorage[j*length+k] * dest(i, k);
		// 		}

		// 		for (size_t j=length+1; j!=(size_t)-1; --j) {
		// 			for (size_t k = j+1; k!=length; ++k)
		// 				dest(i, j) -= TempStorage[j*length+k] * dest(i, k);

		// 			dest(i, j) /= TempStorage[j*(length+1)];
		// 		}
		// 	}
		// }


	} else if (rows(A) == cols(A)){
		invert(dest, A);
	} else{
		invert(dest, A*tr(A));
		dest = tr(A) * cp(dest);
	}
}




template<SP_MATRIX_T(M)>
auto trace(M &&A) noexcept{
	size_t length= min(rows(A), cols(A));
	typename std::decay_t<M>::ValueType result = A(0, 0);
	for (size_t i=1; i!=length; ++i) result += A(i, i);
	return result;
}

template<SP_MATRIX_T(M)>
auto determinant(M &&A) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a determinant");
	size_t length = rows(A);
	
	if constexpr (std::is_rvalue_reference_v<M>){
		typename std::decay_t<M>::ValueType result = typename std::decay_t<M>::ValueType{1};
		typename std::decay_t<M>::ValueType factor1, factor2;
		if constexpr (std::decay_t<M>::RowMajor){
			for (size_t i=0; i<length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(k, i))>abs(A(j, i)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(i, k), A(j, k));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(j, k) -= A(i, k) * factor2;
				}
			}
		} else{
			for (size_t i=0; i<length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(i, k))>abs(A(i, j)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(k, i), A(k, j));
						result = -result;
					}
				}
				factor1 = A(i, i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(k, j) -= A(k, i) * factor2;
				}
			}
		}
		return result * A(length-1, length-1);
	} else{
		size_t oldSize = len(MatrixTempStorage.data);
		expand_back(
			MatrixTempStorage.data,
			(length*length * sizeof(typename std::decay_t<M>::ValueType) + 7) / 8
		);
		typename std::decay_t<M>::ValueType *TempStorage = (
			typename std::decay_t<M>::ValueType *
		)beg(MatrixTempStorage.data) + oldSize;

		{
			typename std::decay_t<M>::ValueType *I = TempStorage;
			for (size_t i=0; i!=length; ++i)
				for (size_t j=0; j!=length; ++j, ++I)
					if constexpr (std::decay_t<M>::RowMajor)
						*I = A(i, j);
					else
						*I = A(j, i);
		}
		
		typename std::decay_t<M>::ValueType result = typename std::decay_t<M>::ValueType{1};
		typename std::decay_t<M>::ValueType factor1, factor2;
		for (size_t i=0; i<length-1; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=length; ++k)	// find row with max value
					j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
				if (j != i){
					for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
						swap(TempStorage[i*length+k], TempStorage[j*length+k]);
					result = -result;
				}
			}
			factor1 = TempStorage[i*(length+1)];
			result *= factor1;
			for (size_t j=i+1; j!=length; ++j){
				factor2 = TempStorage[j*length+i] / factor1;
				for (size_t k=i+1; k!=length; ++k)
					TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
			}
		}
		result *= TempStorage[length*length-1];
		
		resize(MatrixTempStorage.data, oldSize);
		return result;
	}
}

template<SP_MATRIX_T(M)>
auto minor(M &&A, size_t row, size_t col) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a minor");
	size_t length= rows(A) - 1;
	
	if constexpr (std::is_rvalue_reference_v<M>){
		if constexpr (std::decay_t<M>::RowMajor){
			{
				for (size_t i=row; i!=length; ++i)
					for (size_t j=col; j!=length; ++j)
						A(i, j) = A(i+1, j+1);
			}
			
			typename std::decay_t<M>::ValueType result = (typename std::decay_t<M>::ValueType)1;
			typename std::decay_t<M>::ValueType factor1, factor2;
			for (size_t i=0; i!=length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(k, i))>abs(A(j, i)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(i, k), A(j, k));
						result = -result;
					}
				}
				factor1 = A(i ,i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(j, i) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(j, k) -= A(i, k) * factor2;
				}
			}
			return result * A(length-1, length-1);
		} else{
			{
				for (size_t i=col; i!=length; ++i)
					for (size_t j=row; j!=length; ++j)
						A(j, i) = A(j+1, i+1);
			}
			
			typename std::decay_t<M>::ValueType result = (typename std::decay_t<M>::ValueType)1;
			typename std::decay_t<M>::ValueType factor1, factor2;
			for (size_t i=0; i!=length-1; ++i){
				{
					size_t j = i;
					for (size_t k=i+1; k!=length; ++k)	// find row with max value
						j = abs(A(i, k))>abs(A(i, j)) ? k : j;
					if (j != i){
						for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
							swap(A(i, k), A(k, j));
						result = -result;
					}
				}
				factor1 = A(i ,i);
				result *= factor1;
				for (size_t j=i+1; j!=length; ++j){
					factor2 = A(i, j) / factor1;
					for (size_t k=i+1; k!=length; ++k)
						A(k, j) -= A(i, k) * factor2;
				}
			}
			return result * A(length-1, length-1);
		}
	} else{
		size_t oldSize = len(MatrixTempStorage.data);
		expand_back(
			MatrixTempStorage.data,
			(length*length * sizeof(typename std::decay_t<M>::ValueType) + 7) / 8
		);
		typename std::decay_t<M>::ValueType *TempStorage = (
			typename std::decay_t<M>::ValueType *
		)beg(MatrixTempStorage.data) + oldSize;

		{
			typename std::decay_t<M>::ValueType *I = TempStorage;
			for (size_t i=0; i!=row; ++i){	// Make submatrix
				for (size_t j=0; j!=col; ++j, ++I)
					*I = A(i, j);
				for (size_t j=col; j!=length; ++j, ++I)
					*I = A(i, j+1);
			}
			for (size_t i=row+1; i!=length+1; ++i){
				for (size_t j=0; j!=col; ++j, ++I)
					*I = A(i, j);
				for (size_t j=col; j!=length; ++j, ++I)
					*I = A(i, j+1);
			}
		}
		
		typename std::decay_t<M>::ValueType result = (typename std::decay_t<M>::ValueType)1;
		typename std::decay_t<M>::ValueType factor1, factor2;
		for (size_t i=0; i!=length-1; ++i){
			{
				size_t j = i;
				for (size_t k=i+1; k!=length; ++k)	// find row with max value
					j = abs(TempStorage[k*length+i])>abs(TempStorage[j*length+i]) ? k : j;
				if (j != i){
					for (size_t k=i; k!=length; ++k)	// exchange top row with row with max value
						swap(TempStorage[i*length+k], TempStorage[j*length+k]);
					result = -result;
				}
			}
			factor1 = TempStorage[i*(length+1)];
			result *= factor1;
			for (size_t j=i+1; j!=length; ++j){
				factor2 = TempStorage[j*length+i] / factor1;
				for (size_t k=i+1; k!=length; ++k)
					TempStorage[j*length+k] -= TempStorage[i*length+k] * factor2;
			}
		}
		result *= TempStorage[length*length-1];
		
		resize(MatrixTempStorage.data, oldSize);
		return result;
	}
}

template<SP_MATRIX_T(M)>
auto cofactor(M &&A, size_t row, size_t col) noexcept{
	SP_MATRIX_ERROR(rows(A) != cols(A), "only square matrix can have a cofactor");
	auto result = minor((M &&)A, row, col);
	return (row + col)&1 ? -result : result;
}






template<SP_MATRIX_T(M)>
bool is_lower_triangular(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.cols(); ++j)
			if (A(i, j) != (typename std::decay_t<M>::ValueType)0) return false;
	return true;
}

template<SP_MATRIX_T(M)>
bool is_upper_triangular(M &&A) noexcept{
	for (size_t i=0; i!=A.cols(); ++i)
		for (size_t j=i; j!=A.rows(); ++j)
			if (A(j, i) != (typename std::decay_t<M>::ValueType)0) return false;
	return true;
}

template<SP_MATRIX_T(M)>
bool is_symmetric(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.rows(); ++j)
			if (A(i, j) != A(j, i)) return false;
	return true;
}

template<SP_MATRIX_T(M)>
bool is_diagonal(M &&A) noexcept{
	for (size_t i=0; i!=A.rows(); ++i)
		for (size_t j=i+1; j!=A.rows(); ++j)
			if (
				A(i, j)!=(typename std::decay_t<M>::ValueType)0
				&& A(j, i)!=(typename std::decay_t<M>::ValueType)0
			) return false;
	return true;
}


} namespace{


template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
bool operator ==(M1 &&lhs, M2 &&rhs) noexcept{
	if (rows(lhs)!=rows(rhs) || cols(lhs)!=cols(rhs)) return false;
	if constexpr (M1::RowMajor || M2::RowMajor){
		for (size_t i=0; i!=rows(lhs); ++i)
			for (size_t j=0; j!=cols(lhs); ++j)
				if (lhs(i, j) != rhs(i, j)) return false;
	} else{
		for (size_t i=0; i!=cols(lhs); ++i)
			for (size_t j=0; j!=rows(lhs); ++j)
				if (lhs(j, i) != rhs(j, i)) return false;
	}
	return true;
}

template<SP_MATRIX_T(M1), SP_MATRIX_T(M2)>
bool operator !=(M1 &&lhs, const M2 &&rhs) noexcept{ return !(lhs == rhs); }



} // END OF OPEARTORS























namespace sp{



template<SP_VECTOR_T(V)>
constexpr auto rank(V &&A) noexcept{ return 1; }

template<SP_VECTOR_T(V)>
constexpr auto trace(V &&A) noexcept{ return len(A) ? A[0] : (typename std::decay_t<V>::ValueType)0; }

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto inner_prod(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != len(rhs), "operands of inner product cannot have different lengths");
	typename std::decay_t<V1>::ValueType result = (typename std::decay_t<V1>::ValueType)0;
	for (size_t i=0; i!=len(lhs); ++i)
		result += lhs[i] * rhs[i];
	return result;
}

template<auto Operation, SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
auto inner_op(V1 &&lhs, V2 &&rhs) noexcept{
	SP_MATRIX_ERROR(len(lhs) != len(rhs), "operands of \"inner\" operation cannot have different lengths");
	typename std::decay_t<V1>::ValueType result = (typename std::decay_t<V1>::ValueType)0;
	for (size_t i=0; i!=len(lhs); ++i)
		result += Operation(lhs[i], rhs[i]);
	return result;
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2), class Operation>
auto inner_prod(V1 &&lhs, V2 &&rhs, Operation &&operation) noexcept{
	SP_MATRIX_ERROR(len(lhs) != len(rhs), "operands of \"inner\" operation cannot have different lengths");
	typename std::decay_t<V1>::ValueType result = (typename std::decay_t<V1>::ValueType)0;
	for (size_t i=0; i!=len(lhs); ++i)
		result += operation(lhs[i], rhs[i]);
	return result;
}




// template<class Cont>
// void permute(Cont &dest) noexcept{
// 	static_assert(std::is_integral_v<typename Cont::ValueType>, "permutation array must contain integral values");
// 	for (size_t i=0; i!=len(dest)-1; ++i)
// 		while (i != dest[i])
// 			swap(dest[i], dest[dest[i]]);
// }

template<SP_VECTOR_T(V), class Cont>
void permute(V &&dest, const Cont &permuts) noexcept{
	static_assert(std::is_integral_v<typename Cont::ValueType>,
		"permutation array must contain integral values"
	);
	SP_MATRIX_ERROR(len(dest) != len(permuts),
		"size of permuted vecror must be the same as the size of permutation array"
	);

	size_t length= len(permuts);
	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(MatrixTempStorage.data, (length* sizeof(typename Cont::ValueType) + 7) / 8);
	typename Cont::ValueType *TempStorage = (typename Cont::ValueType *)beg(MatrixTempStorage.data) + oldSize;
	
	for (size_t i=0; i!=length; ++i) TempStorage[i] = beg(permuts)[i];
	
	for (size_t i=0; i!=length; ++i)
		while (TempStorage[i] != i){
			size_t swapIndex = TempStorage[i];
			iswap(TempStorage+i, TempStorage+swapIndex);
			swap(dest[swapIndex], dest[i]);
		}

	resize(MatrixTempStorage.data, oldSize);	
}













template<class Cont>
void invert_permuts(Cont &dest) noexcept{
	static_assert(std::is_integral_v<typename Cont::ValueType>, "permutation array must contain integral values");

	size_t length= len(dest);
	size_t oldSize = len(MatrixTempStorage.data);
	expand_back(
		MatrixTempStorage.data,
		(length*length * sizeof(typename Cont::ValueType) + 7) / 8
	);
	typename Cont::ValueType *TempStorage = (
		typename Cont::ValueType *
	)beg(MatrixTempStorage.data) + oldSize;
	
	for (size_t i=0; i!=length; ++i) TempStorage[i] = beg(dest)[i];
	
	for (size_t i=0; i!=length; ++i)
		dest[i] = TempStorage[(size_t)TempStorage[i]];

	resize(MatrixTempStorage.data, oldSize);
}

template<class Cont1, class Cont2>
void invert_permuts(Cont1 &dest, const Cont2 &src) noexcept{
	static_assert(
		std::is_integral_v<typename Cont1::ValueType>
		&& std::is_integral_v<typename Cont2::ValueType>,
		"permutation array must contain integral values"
	);

	resize(dest, len(src));
	for (size_t i=0; i!=len(dest); ++i)
		dest[i] = src[(size_t)src[i]];
}










} namespace{
using namespace sp;



template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)>
bool operator ==(V1 &&lhs, V2 &&rhs) noexcept{
	if (len(lhs) != len(rhs)) return false;
	for (size_t i=0, j; i!=size(lhs); ++i)
		if (lhs[i] != rhs[i]) return false;
	return true;
}

template<SP_VECTOR_T(V1), SP_VECTOR_T(V2)> bool operator !=(V1 &&lhs, V2 &&rhs) noexcept{ return !(lhs == rhs); }


} // END OF NAMESPACE ///////////////////////////////////////////////////////////////////
