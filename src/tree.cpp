#include "NewMatrix/Matrix.hpp"
#include "SPL/Utils.hpp"
#include "time.h"
#include "SPL/Arrays.hpp"

template<class Mat1>
void piruint32_tMatrixD(const Mat1 &A){
	for (uint32_t i=0; i<A.rows(); ++i){
		for (uint32_t j=0; j<A.cols(); ++j)
			pruint32_tf("%8.2lf", A(i, j));
		putchar('\n');
	}
	putchar('\n');
}

double foo(uint32_t r, uint32_t c){ return (double)((r+1) * (c+1)); }


template<class T> struct S{
	S(T a(const uint32_t, const uint32_t)) : arg{a} {}

	T (*arg)(const uint32_t, const uint32_t);
};


int main(){

	sp::DMatrix<double> A;
	sp::DMatrix<double> B;

//	sp::HMatrix<double, 20000> C;

	A.resize(1638, 1638);

	sp::Rand32 rd{clock()};

	A = sp::apply(A, [&rd](double x) mutable{ return rd() % 10; });

//	B = sp::apply(A, [&rd](double x) mutable{ return rd() % 10; });

	uint32_t tries = 1;

	time_t timer = clock();

	for (uint32_t i=0; i<tries; ++i){
		B.resize(A.cols(), A.rows());

/*		{
			for (uint32_t i=0; i<B.rows(); ++i)
				for(uint32_t j=0; j<B.cols(); ++j)
					B(i, j) = A(j, i);
		}
*/
		{
		 	constexpr uint32_t StorageAvalible = sp::CacheAvalible / sizeof(double);
		 	constexpr uint32_t StoragePage = sp::CachePage / sizeof(double);
		 	//constexpr uint32_t blockSize = sp::uint32_tSqrt(StorageAvalible / (1 + 1)) / StoragePage * StoragePage;
		 	constexpr uint32_t blockSize = 16;
		 	uint32_t ii, jj;
		 	for (ii=blockSize; ii<B.rows(); ii+=blockSize){
		 		for (jj=blockSize; jj<B.cols(); jj+=blockSize)
		 			for(uint32_t i=ii-blockSize; i<ii; ++i)
		 				for(uint32_t j=jj-blockSize; j<jj; ++j)
		 					B(i, j) = A(j, i);
		 		for(uint32_t i=ii-blockSize; i<ii; ++i)
		 			for(uint32_t j=jj-blockSize; j<B.cols(); ++j)
		 				B(i, j) = A(j, i);
		 	}
		 	for (jj=blockSize; jj<B.cols(); jj+=blockSize)
		 		for(uint32_t i=ii-blockSize; i<B.rows(); ++i)
		 			for(uint32_t j=jj-blockSize; j<jj; ++j)
		 				B(i, j) = A(j, i);
		 	for(uint32_t i=ii-blockSize; i<B.rows(); ++i)
		 		for(uint32_t j=jj-blockSize; j<B.cols(); ++j)
		 			B(i, j) = A(j, i);
		}

/*		{
		 	constexpr uint32_t blockSize = 16;
		 	for (uint32_t jj = 0; jj < B.cols(); jj += blockSize)
		 		for (uint32_t ii = 0; ii < B.rows(); ii += blockSize)
		 			for(uint32_t i = ii; (i < B.rows()) && (i < ii + blockSize); ++i){
		 				uint32_t d;
		 				double temp;
						for(uint32_t j = jj; (j < B.cols()) && (j < jj + blockSize); ++j){
		 					if (i != j){
		 						B(i, j) = A(j, i);
		 					} else{
		 						temp = A(i, j);
		 						d = i;
		 					}
		 				}

		 				if(ii == jj) 
		 					B(d, d) = temp;
		 			}
		}
*/	}


	// piruint32_tMatrixD(A);
	// piruint32_tMatrixD(B);
//	piruint32_tMatrixD(C);

	printf("B[0, 1] = %lf\ntime: %ld\n", B(0, 1), clock()-timer);

	return 0;
}
