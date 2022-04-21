#define SP_MATRIX_DEBUG
#include "matrix/Initializators.hpp"

#include "SPL/Scans.hpp"


using Mallocator = sp::MallocAllocator<>;

template<bool RowMajor = true>
using Matrix = sp::MatrixWrapper<sp::MatrixDynamic<double, RowMajor, Mallocator>>;
using Vector = sp::VectorWrapper<sp::VectorDynamic<double, Mallocator>>;
using Permutations = sp::DynamicArray<uint32_t, Mallocator>;

template<class M> void print_matrix(const M &m);
template<class V> void print_vector(const V &v);
Matrix<> scan_matrix(FILE *input);
Vector scan_vector(FILE *input);



int main(){
	Matrix A = scan_matrix(fopen("matrix.txt", "r"));
//	Matrix B = scan_matrix();
	Matrix C;

//	Vector V = scan_vector();

//	print_vector(V);
	
//	sp::permute_rows(A, perms);
	
//	puts("outer_product:");
//	print_matrix(sp::outer_prod(sp::as_row(A, 1), sp::as_col(A, 0)));

//	printf("\ninner_product: %lf\n", sp::inner_prod(sp::as_row(A, 1), sp::as_col(A, 0)));
	print_matrix(A);

	printf("\ndeterminant: %lf\n", sp::determinant(A));
	return 0;
}

















template<class M>
void print_matrix(const M &m) noexcept{
	for (size_t i=0; i!=sp::rows(m); ++i){
		for (size_t j=0; j!=sp::cols(m); ++j)
			printf("%7.2lf", (double)m(i, j));
		putchar('\n');
	}
	putchar('\n');
}

template<class V>
void print_vector(const V &v) noexcept{
	for (size_t i=0; i!=sp::len(v); ++i)
		printf("%7.2lf", (double)v[i]);
	putchar('\n');
	putchar('\n');
}

Matrix<> scan_matrix(FILE *input = stdin) noexcept{
	sp::DynamicArray<double, Mallocator> arr;
	auto[r, c, last, err] = sp::scan_matrix(arr, input);
	SP_MATRIX_ERROR(err, "wrong input format for matrix");
	return Matrix<>{{arr.data, r, c}};
}

Vector scan_vector(FILE *input = stdin) noexcept{
	sp::DynamicArray<double, Mallocator> arr;
	auto[size, last] = sp::scan_array(arr, input);

	return Vector{{arr.data, arr.size}};
}


