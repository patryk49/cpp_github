#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <stdlib.h>


// SCANNIG FOR NUMBERS
template<class T>
struct ScanNumberRes{ T res; char last; bool not_found; };

template<class T> static
ScanNumberRes<T> scan_number(FILE *input) noexcept{
	ScanNumberRes<T> res;
	res.not_found = false;

	do{
		res.last = getc(input);
	} while (res.last==' ' || res.last=='\t');

	if constexpr (std::is_integral_v<T>){
		if constexpr (std::is_unsigned_v<T>){
			if (res.last<'0' || res.last>'9'){
				res.not_found = true;
				return res;
			}
	
			res.res = (T)((int8_t)res.last - '0');
			for (;;){
				res.last = getc(input);
				if (res.last<'0' || res.last>'9') return res;
				res.res = res.res*10 + (T)((int8_t)res.last-'0');
			}
		} else{
			bool neg = false;
			if (res.last=='+' || res.last=='-'){
				neg = res.last == '-';
				res.last = getc(input);
			}

			if (res.last<'0' || res.last>'9'){
				res.not_found = true;
				return res;
			}

			res.res = (T)((int8_t)res.last - '0');
			for (;;){
				res.last = getc(input);
				if (res.last<'0' || res.last>'9'){
					if (neg) res.res = -res.res;
					return res;
				}
				res.res = res.res*10 + (T)((int8_t)res.last-'0');
			}
		}
	} else if constexpr (std::is_floating_point_v<T>){
		bool neg = false;


		if (res.last=='+' || res.last=='-'){
			neg = res.last == '-';
			res.last = getc(input);
		}
		
		if (res.last<'0' || res.last>'9'){
			res.not_found = true;
			return res;
		}

		res.res = (T)((int8_t)res.last - '0');
		for (;;){
			res.last = getc(input);
			if (res.last == '.') break;
			if (res.last<'0' || res.last>'9'){
				if (neg) res.res = -res.res;
				return res;
			}
			res.res = res.res*10.0 + (T)((int8_t)res.last-'0');
		}
		
		for (double scale = 0.1;; scale *= 0.1){
			res.last = getc(input);
			if (res.last<'0' || res.last>'9'){
				if (neg) res.res = -res.res;
				return res;
			}
			res.res += (T)((int8_t)res.last-'0') * scale;
		}
	} else{
		static_assert(true, "wrong type");
	}
}



struct ScanArrayRes{ uint32_t size; char last; };

template<class Cont> static
ScanArrayRes scan_array(
	Cont &arr, FILE *input, const char separator = ' '
) noexcept{
	using T = typename Cont::ValueType;
	ScanNumberRes<T> num;

	uint32_t n = 0;
	for (;;){
		num = scan_number<T>(input);
		[[unlikely]] if (num.not_found) break;
		push_value(arr, num.res);
		++n;
		if (num.last != separator) break;
	}
	return ScanArrayRes{n, num.last};
}


struct ScanMatrixRes{ uint32_t rows, cols; char last; bool col_mismatch; };

template<class Cont> static
ScanMatrixRes scan_matrix(
	Cont &arr, FILE *input, const char col_separator = ' '
) noexcept{
	ScanMatrixRes res{0, 0, '\0', false};
	ScanArrayRes arr_info;

	arr_info = scan_array(arr, input, col_separator);
	res.cols = arr_info.size;
	[[unlikely]] if (!arr_info.size) goto Return;

	for (;;){
		++res.rows;
		arr_info = scan_array(arr, input, col_separator);
		
		if (!arr_info.size) break;
		[[unlikely]] if (arr_info.size != res.cols){
			res.col_mismatch = true;
			res.rows += arr_info.size > res.cols;
			break;
		}		
	}
Return:
	res.last = arr_info.last;
	return res;
}

