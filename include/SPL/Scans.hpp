#include "SPL/Arrays.hpp"
#include <stdio.h>
#include <stdlib.h>


// SCANNIG FOR NUMBERS
template<class A>
Array<char> read_text(FILE *input, A &allocator, size_t expected_length = 64) noexcept{
	Array<char> res = {alloc(allocator, expected_length), 0};
	
	for (;;){
		int c = getc(input);
		[[unlikely]] if (c == EOF) break;

		if (res.size == res.cap){
			auto[new_ptr, new_cap] = realloc(allocator, Memblock{res.ptr, res.cap}, res.size*2);
			if (new_ptr == nullptr){
				free(allocator, Memblock{res.ptr, res.cap});
				return Array<char>{nullptr, 0, 0};
			}
			res.ptr = new_ptr;
			res.cap = new_cap;
		}

		res[res.size] = (char)c;
		++res.size;
	}
	return res;
}

/*
template<class A>
Array<Span<char>> scan_text(FILE *input, A &allocator, Span<char> delim) noexcept{
	Array<char> res = {alloc(allocator, 128), 0};

	uint8_t delim_counter[32];
	for (auto &it : delim_counter) it = 0;

	for (;;){
		int c = getc(input);
		[[unlikely]] if ((char)c==delim || c==EOF) break;

		if (res.size == res.data.size){
			Memblock new_data = realloc(res.data, allocator, res.size*2);
			if (new_data.ptr == nullptr){
				free(allocator, res.data);
				return Array<char>{{nullptr, 0}, 0};
			}
			res.data = new_data;
		}

		res[res.size] = (char)c;
		++res.size;
	}
	return res;
}
*/

template<class T>
struct ReadDataRes{ T res; bool not_found; char last; };

template<class T> static
ReadDataRes<T> read_data(FILE *input) noexcept{
	ReadDataRes<T> res;
	res.not_found = false;


	if constexpr (std::is_integral_v<T>){
		do{ res.last = getc(input); } while (res.last==' ' || res.last=='\t');
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
		do{ res.last = getc(input); } while (res.last==' ' || res.last=='\t');
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
	} else if constexpr(is_tuple<T>){
		auto[head_data, err, last] = read_data<decltype(res.res.head)>(input);
		res.last = last;
		if (err){ res.not_found = true; return res; }
		res.res.head = head_data;

		if constexpr (len(res.res) != 1){
			auto[tail_data, err, last] = read_data<decltype(res.res.tail)>(input);
			res.last = last;
			if (err){ res.not_found = true; return res; }
			res.res.tail = tail_data;
		}
		
		return res;
	} else{
		static_assert(true, "wrong type");
	}
}

template<class T> struct ReadArrayRes{
	Array<T> res = {nullptr, 0, 0};
	bool alloc_error = false;
	char last;
};

template<class T, class A> static
ReadArrayRes<T> read_array(FILE *input, A &allocator, char separator = ' ') noexcept{
	ReadArrayRes<T> res = {};
	ReadDataRes<T> elem;
	for (;;){
		elem = read_data<T>(input);
		[[unlikely]] if (elem.not_found) break;
		
		res.alloc_error = push_value(res.res, elem.res, allocator);
		if (res.alloc_error) break;
		
		if (elem.last != separator) break;
	}
	return res;
}

/*
struct ScanMatrixRes{ uint32_t rows; uint32_t cols; char last; uint16_t col_mismatch; };

template<class Cont> static
ScanMatrixRes scan_matrix(Cont &arr, FILE *input, char col_separator = ' ') noexcept{
	ScanMatrixRes res{0, 0, '\0', 0};
	ReadArrayRes arr_info;

	arr_info = scan_array(arr, input, col_separator);
	res.cols = arr_info.size;
	[[unlikely]] if (!arr_info.size) goto Return;

	for (;;){
		++res.rows;
		arr_info = scan_array(arr, input, col_separator);
		
		if (!arr_info.size) break;
		[[unlikely]] if (arr_info.size != res.cols){
			res.col_mismatch = arr_info.size;
			break;
		}
	}
Return:
	res.last = arr_info.last;
	return res;
}
*/
