#pragma once

#include <type_traits>
#include <bit>

#include <stddef.h>
#include <stdint.h>

//#pragma GCC diagnostic push
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define SP_CSI constexpr inline
#define SP_SI static inline
#define SP_CI constexpr inline

namespace sp{ // BEGINING OF NAMESPACE ////////////////////////////////////////////////////////////


#ifdef SP_DEBUG
	static constexpr bool DebugMode = true;
#else
	static constexpr bool DebugMode = false;
#endif


template<class L>
struct DefererImpl{
	L lambda;
	DefererImpl(L l) noexcept : lambda{l} {}
};

#define SP_CAT_IMPL(x, y) x ## y
#define SP_CAT(x, y) SP_CAT_IMPL(x, y)

#define SP_DEFER ::sp::DefererImpl SP_CAT(_sp_defer_, __LINE__) = [&]() mutable






// TYPE TRAITS
namespace priv__{
	template<uint32_t size> struct IntOfGivenSizeStruct;
	template<> struct IntOfGivenSizeStruct<1>{ typedef int8_t type; };
	template<> struct IntOfGivenSizeStruct<2>{ typedef int16_t type; };
	template<> struct IntOfGivenSizeStruct<4>{ typedef int32_t type; };
	template<> struct IntOfGivenSizeStruct<8>{ typedef int64_t type; };
	template<> struct IntOfGivenSizeStruct<16>{ typedef int64_t type; };

	template<uint32_t size> struct UIntOfGivenSizeStruct;
	template<> struct UIntOfGivenSizeStruct<1>{ typedef uint8_t type; };
	template<> struct UIntOfGivenSizeStruct<2>{ typedef uint16_t type; };
	template<> struct UIntOfGivenSizeStruct<4>{ typedef uint32_t type; };
	template<> struct UIntOfGivenSizeStruct<8>{ typedef uint64_t type; };
	template<> struct UIntOfGivenSizeStruct<16>{ typedef uint64_t type; };
}

template<uint32_t size> using IntOfGivenSize =
	typename priv__::UIntOfGivenSizeStruct<size>::type;
template<uint32_t size> using UIntOfGivenSize =
	typename priv__::UIntOfGivenSizeStruct<size>::type;

template<class T>
constexpr bool isTriviallyPassabe = sizeof(T)<=16 && std::is_trivially_constructible_v<T>;

template<class T> struct CRemRRefStruct{ typedef T type; };
template<class T> struct CRemRRefStruct<T &&>{ typedef T type; };
template<class T> struct CRemRRefStruct<T &>{ typedef const T &type; };
template<class T> using CRemRRef = typename CRemRRefStruct<T>::type;

template<class T> struct RemRRefStruct{ typedef T type; };
template<class T> struct RemRRefStruct<T &&>{ typedef T type; };
template<class T> using RemRRef = typename RemRRefStruct<T>::type;


// POINTER HELPERS
SP_SI uint8_t *align(void *ptr, size_t alignment) noexcept{
	return (uint8_t *)(((uintptr_t)ptr - 1u + alignment) & -alignment);
}
SP_SI const void *align(const void *ptr, size_t alignment) noexcept{
	return (const uint8_t *)(((uintptr_t)ptr - 1u + alignment) & -alignment);
}

template<class T> SP_CSI T &deref(T &arg) noexcept{ return arg; }
template<class T> SP_CSI T &deref(T *arg) noexcept{ return *arg; }
template<class T> SP_CSI const T &deref(const T &arg) noexcept{ return arg; }
template<class T> SP_CSI const T &deref(const T *arg) noexcept{ return *arg; }



// OVERLOADS FOR OPERATORS
#define SP_BINARY_OP_MACRO(op) { \
	SP_CI decltype(auto) operator ()(TL lhs, TR rhs) const noexcept{ return lhs op rhs; } \
};

template<class TL, class TR = TL> struct Op_Plus			SP_BINARY_OP_MACRO(+)
template<class TL, class TR = TL> struct Op_Minus		  SP_BINARY_OP_MACRO(-)
template<class TL, class TR = TL> struct Op_Multiply	  SP_BINARY_OP_MACRO(*)
template<class TL, class TR = TL> struct Op_Divide		 SP_BINARY_OP_MACRO(/)
template<class TL, class TR = TL> struct Op_Modulo		 SP_BINARY_OP_MACRO(%)

template<class TL, class TR = TL> struct Op_Lesser		 SP_BINARY_OP_MACRO(<)
template<class TL, class TR = TL> struct Op_Greater		SP_BINARY_OP_MACRO(>)
template<class TL, class TR = TL> struct Op_LesserEqual  SP_BINARY_OP_MACRO(<=)
template<class TL, class TR = TL> struct Op_GreaterEqual SP_BINARY_OP_MACRO(>=)
template<class TL, class TR = TL> struct Op_Equal		  SP_BINARY_OP_MACRO(==)
template<class TL, class TR = TL> struct Op_NotEqual	  SP_BINARY_OP_MACRO(!=)
template<class TL, class TR = TL> struct Op_LogicOr		SP_BINARY_OP_MACRO(||)
template<class TL, class TR = TL> struct Op_LogicAnd	  SP_BINARY_OP_MACRO(&&)

template<class TL, class TR = TL> struct Op_BitOr		  SP_BINARY_OP_MACRO(|)
template<class TL, class TR = TL> struct Op_BitAnd		 SP_BINARY_OP_MACRO(&)
template<class TL, class TR = TL> struct Op_BitXor		 SP_BINARY_OP_MACRO(^)
template<class TL, class TR = TL> struct Op_LeftShift	 SP_BINARY_OP_MACRO(<<)
template<class TL, class TR = TL> struct Op_RightShift	SP_BINARY_OP_MACRO(>>)

#undef SP_BINARY_OP_MACRO
#define SP_UNARY_OP_MACRO(op) { \
	SP_CI decltype(auto) operator ()(T arg) const noexcept{ return op arg; } \
};

template<class T> struct Op_UnaryPlus	SP_UNARY_OP_MACRO(+)
template<class T> struct Op_UnaryMinus  SP_UNARY_OP_MACRO(-)
template<class T> struct Op_LogicNot	 SP_UNARY_OP_MACRO(!)
template<class T> struct Op_BitNot		SP_UNARY_OP_MACRO(~)
template<class T> struct Op_Dereference SP_UNARY_OP_MACRO(*)

#undef SP_UNARY_OP_MACRO




// DEFAULT INITIALIZATIONS AND DEINITIALIZATIONS
template<class T> SP_CSI void init(T &arg) noexcept{ arg = T{}; }
template<class T> SP_CSI void deinit(T &range) noexcept{}
template<class T> constexpr bool needs_init = false;
template<class T> constexpr bool needs_deinit = false;

// COMMON OPERATIONS FOR ARRAYS
template<class T, size_t N> SP_CSI size_t len(const T (&)[N]) noexcept{ return N; }
template<class T, size_t N> SP_CSI T *beg(T (&arr)[N]) noexcept{ return arr; }
template<class T, size_t N> SP_CSI T *end(T (&arr)[N]) noexcept{ return arr + N; }
template<class T, size_t N> SP_CSI T &front(T (&arr)[N]) noexcept{ return *arr; }
template<class T, size_t N> SP_CSI T &back(T (&arr)[N]) noexcept{ return *(arr + N - 1); }
template<class T, size_t N> constexpr bool needs_init<T (&)[N]> = needs_init<T>;
template<class T, size_t N> constexpr bool needs_deinit<T (&)[N]> = needs_deinit<T>;

template<class T, size_t N> SP_CSI void init(T (&arr)[N]) noexcept{
	for (T *I=arr; I!=arr+N; ++I) init(*I);
}

template<class T, size_t N> SP_CSI void deinit(T (&arr)[N]) noexcept{
	for (T *I=arr; I!=arr+N; ++I) deinit(*I);
}

template<class TD, class TS>
SP_CSI void copy(TD &dest, const TS &src) noexcept{
	if constexpr (std::is_array_v<TS>)
		for (size_t i=0; i!=sp::len(src); ++i) copy(dest[i], src[i]);
	else
		dest = src;
}



// GENERIC MATH CONSTANTS
template<class T> static constexpr T Null = T{};
template<class T> static constexpr T Unit = (T)1;


// COMMON SIMPLE GENERIC FUNCTIONS
template<class T> T &min(T &x, T &y) noexcept{ return x>y ? y : x; }
template<class T> const T &min(const T &x, const T &y) noexcept{ return x>y ? y : x; }

template<class T> T &max(T &x, T &y) noexcept{ return x<y ? y : x; }
template<class T> const T &max(const T &x, const T &y) noexcept{ return x<y ? y : x; }


// SIMPLE RANGE CLASS
template<class T>
struct Range{
	SP_CI T &operator[](size_t index) noexcept{ return *(ptr + index); }
	SP_CI const T &operator[](size_t index) const noexcept{ return *(ptr + index); }

	SP_CI operator Range<uint8_t>() const noexcept{
		return Range<uint8_t>{(uint8_t *)ptr, (size*sizeof(T))};
	}
	SP_CI operator Range<std::add_const_t<T>>() const noexcept{
		return Range<std::add_const_t<T>>{
			(std::add_pointer_t<std::add_const_t<T>>)ptr, 
			size
		};
	}

	typedef T ValueType;

	T *ptr;
	size_t size;
};

template<class T> SP_CSI size_t len(Range<T> range) noexcept{ return range.size; }
template<class T> SP_CSI T *beg(Range<T> range) noexcept{ return range.ptr; }
template<class T> SP_CSI T *end(Range<T> range) noexcept{ return range.ptr + range.size; }
template<class T> SP_CSI T &front(Range<T> range) noexcept{ return *range.ptr; }
template<class T> SP_CSI T &back(Range<T> range) noexcept{ return *(range.ptr + range.size - 1); }

template<class T> SP_CSI void pop(Range<T> &range) noexcept{ --range.size; }
template<class T> SP_CSI void pop_front(Range<T> &range) noexcept{ ++range.ptr; }

template<class T>
SP_CSI T &pop_val(Range<T> &range) noexcept{
	return *(range.ptr + --range.size);
}

template<class T>
SP_CSI T &pop_front_val(Range<T> &range) noexcept{
	T *res = range.ptr;
	++range.ptr;
	return *res;
}

template<class T> void resize(Range<T> &range, size_t size) noexcept{ range.size = size; }
template<class T> void shrink_back(Range<T> &range, size_t size) noexcept{ range.size -= size; }
template<class T> void shrink_front(Range<T> &range, size_t size) noexcept{ range.ptr += size; }


template<class Cont>
SP_CSI auto range(Cont &cont) noexcept{
	return Range<std::remove_reference_t<decltype(cont[0])>>{beg(cont), len(cont)};
}
template<size_t N>
SP_CSI auto range(char (&str)[N]) noexcept{
	return Range<char>{(char *)str, N-1};
}
template<size_t N>
SP_CSI auto range(const char (&str)[N]) noexcept{
	return Range<const char>{(const char *)str, N-1};
}
template<class Cont>
SP_CSI auto range(Cont &cont, size_t from, size_t to) noexcept{
	return Range<std::remove_reference_t<decltype(cont[0])>>{beg(cont)+from, to-from};
}
template<class T>
SP_CSI auto range(T *first, T *last) noexcept{
	return Range<T>{first, last-first};
}

template<class TL, class TR>
SP_CSI bool operator ==(Range<TL> lhs, Range<TR> rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const TL *sent = lhs.ptr + lhs.size;
	const TR *J = rhs.ptr;
	for (const TL *I=lhs.ptr; I!=sent; ++I, ++J) if (*I != *J) return false;
	return true;
}

template<class TL, class TR>
SP_CSI bool operator !=(Range<TL> lhs, Range<TR> rhs) noexcept{
	return !(lhs == rhs);
}

template<class T>
SP_CSI void init(Range<T> range) noexcept{
	for (T *I=range.ptr; I!=range.ptr+range.size; ++I) init(*I);
}

template<class T>
SP_CSI void deinit(Range<T> range) noexcept{
	for (T *I=range.ptr; I!=range.ptr+range.size; ++I) deinit(*I);
}

using Memblock = Range<uint8_t>;



// MOVING THE VALUES
template<class T>
SP_CSI void swap(T &lhs, T &rhs) noexcept{
	T temp = lhs;
	lhs = rhs;
	rhs = temp;
}

template<class T>
SP_CSI void iswap(T *lhs, T *rhs) noexcept{
	T temp = *lhs;
	*lhs = *rhs;
	*rhs = temp;
}





// INTEGER MATH
namespace priv__{
constexpr uint32_t Uint32LogLookup[32] = {
	  0,  9,  1, 10, 13, 21,  2, 29,
	 11, 14, 16, 18, 22, 25,  3, 30,
	  8, 12, 20, 28, 15, 17, 24,  7,
	 19, 27, 23,  6, 26,  5,  4, 31
};
constexpr uint64_t Uint64LogLookup[64] = {
	 63,  0, 58,  1, 59, 47, 53,  2,
	 60, 39, 48, 27, 54, 33, 42,  3,
	 61, 51, 37, 40, 49, 18, 28, 20,
	 55, 30, 34, 11, 43, 14, 22,  4,
	 62, 57, 46, 52, 38, 26, 32, 41,
	 50, 36, 17, 19, 29, 10, 13, 21,
	 56, 45, 25, 31, 35, 16,  9, 12,
	 44, 24, 15,  8, 23,  7,  6,  5
};
} // END OF NAMESPACE PRIV //////////

SP_CSI uint32_t logb(uint32_t x) noexcept{
	 x |= x >> 1;
	 x |= x >> 2;
	 x |= x >> 4;
	 x |= x >> 8;
	 x |= x >> 16;
	 return priv__::Uint32LogLookup[(uint32_t)(x*0x07c4acdd) >> 27];
}

SP_CSI uint64_t logb(uint64_t x) noexcept{
	 x |= x >> 1;
	 x |= x >> 2;
	 x |= x >> 4;
	 x |= x >> 8;
	 x |= x >> 16;
	 x |= x >> 32;
	 return priv__::Uint64LogLookup[((uint64_t)((x - (x >> 1))*0x07edd5e59a4e28c2)) >> 58];
}

template<class T>
SP_CSI T abs(T x) noexcept{ return (x < (T)0) ? -x : x; }

template<class T>
SP_CSI T sign(T x) noexcept{
	if (x == (T)0) return (T)0;
	return x < (T)0 ? (T)-1 : (T)1;
}


template<class T>
SP_CSI T gcd(T m, T n) noexcept{       // copied from libstdc++
	static_assert(std::is_unsigned_v<T>, "type must be unsigned");
	
	if (m == 0) return n;
	if (n == 0) return m;

	int i = std::countr_zero(m);
	m >>= i;
	int j = std::countr_zero(n);
	n >>= j;
	int k = i < j ? i : j;

	for (;;){
		if (m > n){
			T t = m;
			m = n;
			n = t;
		}
		n -= m;
		if (n == 0) return m << k;

		n >>= std::countr_zero(n);
	}
}

template<class T> // copied from libstdc++
SP_CSI T lcm(T m, T n) noexcept{
	return m && n ? (m / gcd(m, n))*n : 0;
}


SP_CSI size_t factorial(size_t n) noexcept{
	uint32_t result = n;
	while (--n) result *= n;
	return result;
}

template<class T>
SP_CSI std::enable_if_t<std::is_integral_v<T>, T> int_sqrt(T x) noexcept{
	T one = 1 << 30;
	while (one > x) one >>= 2;
	T res = 0;
	while (one){
		T cond = -(x >= res + one);
		x -= (res + one) & cond;
		res += (one << 1) & cond;

		res >>= 1;
		one >>= 2;
	}
	return res;
}




// FLOATING POINT MATH
SP_CSI int32_t get_mantissa(float x) noexcept{
	if (x < 0.f)
		return -((1<<23) | (std::bit_cast<int32_t>(x) & 0x007fffff));
	return (1<<23) | (std::bit_cast<int32_t>(x) & 0x007fffff);
}

SP_CSI int32_t get_exponent(float x) noexcept{
	return ((std::bit_cast<int32_t>(x) & 0x7fffffff) >> 23) - 0x7f;
}

SP_CSI int64_t get_mantissa(double x) noexcept{
	if (x < 0.0)
		return -((1ll<<52) | (std::bit_cast<int64_t>(x) & 0x000fffffffffffff));
	return (1ll<<52) | (std::bit_cast<int64_t>(x) & 0x000fffffffffffff);
}

SP_CSI int64_t get_exponent(double x) noexcept{
	return ((std::bit_cast<int64_t>(x) & 0x7fffffffffffffff) >> 52) - 0x3ff;
}








// CONDITIONAL ARITMETIC
template<class T>
SP_CSI std::enable_if_t<std::is_arithmetic_v<T>, T>
enable(bool condition, T value) noexcept{
	return -(IntOfGivenSize<std::bit_ceil(sizeof(T))>)condition & value;
}

template<class T>
SP_CSI std::enable_if_t<std::is_arithmetic_v<T>, T>
choose(bool condition, T valueOnTrue, T valueOnFalse) noexcept{
	return (
		-(IntOfGivenSize<std::bit_ceil(sizeof(T))>)condition & valueOnTrue) |
		(((IntOfGivenSize<std::bit_ceil(sizeof(T))>)condition-1) & valueOnFalse
	);
}







// TYPE ERASURES
template<class Signature> struct Delegate;

template<class Res, class... Args>
struct Delegate<Res(Args...)>{
	template<class Proc>
	SP_CI Delegate(const Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
	}
	template<class Proc>
	SP_CI Delegate(Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
	}

	template<class Proc>
	SP_CI Delegate(const Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
	}
	template<class Proc>
	SP_CI Delegate(Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
	}
	
	template<class Proc>
	SP_CI Delegate &operator =(const Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
		return *this;
	}
	template<class Proc>
	SP_CI Delegate &operator =(Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
		return *this;
	}

	template<class Proc>
	SP_CI Delegate &operator =(const Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
		return *this;
	}
	template<class Proc>
	SP_CI Delegate &operator =(Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
		return *this;
	}
	
	
	SP_CI Delegate() noexcept = default;
	SP_CI Delegate(const Delegate &) noexcept = default;
	SP_CI Delegate(Delegate &) noexcept = default;
	SP_CI Delegate &operator =(const Delegate &) noexcept = default;
	SP_CI Delegate &operator =(Delegate &) noexcept = default;
	SP_CI Delegate(void *proc, const void *obj) noexcept : procedure{proc}, object{obj} {}

	SP_CI Res operator ()(Args... args) noexcept{
		return object ? (
			((Res (*)(const void *, Args...))procedure)(object, args...)
		) : (
			((Res (*)(Args...))procedure)(args...)
		);
	}

	typedef Res ResultType;

	void *procedure;
	const void *object;
};



// FAST MATH
struct Rand32{
	SP_CI uint32_t operator ()() noexcept{
		seed += 0xe120fc15;
		uint64_t temp = (uint64_t)seed * 0x4a39b70d;
		temp = (uint64_t)((temp >> 32) ^ temp) * 0x12fad5c9;
		return (temp >> 32) ^ temp;
	}

	typedef uint32_t ValueType;

	uint32_t seed;
};

SP_CSI uint32_t min_val(Rand32) noexcept{ return 0; }
SP_CSI uint32_t max_val(Rand32) noexcept{ return UINT32_MAX; }






SP_CSI float qlog(float x) noexcept{
	constexpr float scaleDown = 1.f/(float)0x00800000;
	return (float)(std::bit_cast<uint32_t>(x) - std::bit_cast<uint32_t>(1.f))*scaleDown;
}

SP_CSI float qexp(float x) noexcept{
	constexpr float scaleUp = 0x00800000;
	return std::bit_cast<float>((uint32_t)(x*scaleUp) + std::bit_cast<uint32_t>(1.f));
}

SP_CSI float qpow(float x, float y) noexcept{
	return std::bit_cast<float>(
		(uint32_t)(
			std::bit_cast<uint32_t>(x) - std::bit_cast<uint32_t>(1.f)
		)*y + std::bit_cast<uint32_t>(1.f)
	);
}

SP_CSI float qsqrt(float x) noexcept{
	uint32_t i = std::bit_cast<uint32_t>(x);
	i -= 1<<23;
	i >>= 1;
	i += 1<<29;
	float f = std::bit_cast<float>(i);
	return (f + x/f)*0.5f;
}

SP_CSI float qinv_sqrt(float x) noexcept{
	float f = std::bit_cast<float>((uint32_t)0x5f3759df - (std::bit_cast<uint32_t>(x)>>1));
	return f * (1.5f - 0.5f*x*f*f);
}






// HEAP
template<class T>
SP_CSI void repair_heap(T *begin, T *end, size_t startingIndex) noexcept{
	T *child;
	for(size_t i=startingIndex; (child=begin+((i<<1)|1)) < end; i=child-begin){
		child += child[0] < child[child+1 != end];
		
		if (child[0] < begin[i]) return;

		iswap(begin+i, child);
	}
}

template<class T, class Compare>
SP_CSI void repair_heap(T *begin, T *end, size_t startingIndex, Compare compare) noexcept{
	T *child;
	for(size_t i=startingIndex; (child=begin+((i<<1)|1)) < end; i=child-begin){
		child += compare(child[0], child[child+1 != end]);
		
		if (compare(child[0], begin[i])) return;

		iswap(begin+i, child);
	}
}

template<class T>
SP_CSI void make_heap(T *begin, T *end) noexcept{
	for (size_t i=(end-begin-1)>>1; i!=(size_t)-1; --i)
		repair_heap(begin, end, i);
}

template<class T, class Compare>
SP_CSI void make_heap(T *begin, T *end, Compare compare) noexcept{
	for (size_t i=(end-begin-1)>>1; i!=(size_t)-1; --i)
		repair_heap(begin, end, i, compare);
}








// SOME ALGORITHMS
template<class T, class R>
SP_CSI void shuffle(Range<T> range, R rng) noexcept{
	if (range.size <= 1) return;
	
	for (size_t i=range.size; i!=1;){
		T *rand = range.ptr + rng() % i;
		--i;
		sp::iswap(range.ptr + i, rand);
	}
}

template<class T>
SP_CSI T *partition(Range<T> range, T *pivot) noexcept{
	if (!range.size) return range.ptr;

	T *first = range.ptr;
	T *last = range.ptr + range.size;

	iswap(first, pivot);
	
	T lastVal = (T &&)(*--last);
	*last = *first;
	
	T *It = first;
	for (;;){
		do ++It; while (*It < *first);
		*last = (T &&)(*It);
		do --last; while (*first < *last);
		if (It >= last) break;
		*It = (T &&)(*last);
	}
	if (It == last+2){
		*It = (T &&)(*(last+1));
		--It;
	}
	{
		T *partitionPoint = It - (*first < lastVal);
		*It = (T &&)(lastVal);
		iswap(first, partitionPoint);
		return partitionPoint;
	}
}



}  // END OF NAMESPACE	///////////////////////////////////////////////////////////////////



// THIS DOES NOTHING

/*
// RADIX SORT FOR UNSIGNED TYPES
template<class T, size_t baseBits = 8>
SP_CSI void radixLSD(T *first, T *last) noexcept{
	static_assert(baseBits);
	constexpr size_t base = 1 << baseBits;
	constexpr size_t bitMask = base - 1;
	
	uint32_t counts[base];
	std::vector<T> buffer(last-first); // since it allocates memory it has no place in here

	using IT = sp::UIntOfGivenSize<sp::roundUpPow2(sizeof(T))>;

	for (IT step=0; step!=sizeof(T)*8/baseBits; ++step){
		memset(counts, 0, sizeof(counts));
		if (step & 1){
			for (T *It=&*std::begin(buffer); It!=&*std::end(buffer); ++It)
				++counts[(*(IT *)It >> step*baseBits) & bitMask];

			for (IT *C=std::begin(counts)+1; C!=std::end(counts); ++C) *C += *(C-1);
			
			for (T *It=&*std::end(buffer)-1; It!=&*std::begin(buffer)-1; --It)
				first[--counts[(*(IT *)It >> step*baseBits) & bitMask]] = *It;

		} else{
			for (T *It=first; It!=last; ++It)
				++counts[(*(IT *)It >> step*baseBits) & bitMask];

			for (IT *C=std::begin(counts)+1; C!=std::end(counts); ++C) *C += *(C-1);
			
			for (T *It=last-1; It!=first-1; --It)
				buffer[--counts[(*(IT *)It >> step*baseBits) & bitMask]] = *It;
		}
	}
}
*/


