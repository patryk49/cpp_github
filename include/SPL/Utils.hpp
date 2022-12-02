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
#pragma GCC diagnostic ignored "-Waddress"




#ifdef DEBUG_MODE
	#include <stdio.h>
	static constexpr bool DebugMode = true;
#else
	static constexpr bool DebugMode = false;
#endif



// NAME CONCATENATION MACRO
#define _impl_NAMECAT(x, y) x ## y
#define NAMECAT(x, y) _impl_NAMECAT(x, y)



// TYPE TRAIT FOR GETTING INTEGERS OF WANTED SIZE
template<size_t size> struct _impl_IntOfGivenSize;
template<> struct _impl_IntOfGivenSize<1>{ typedef int8_t type; };
template<> struct _impl_IntOfGivenSize<2>{ typedef int16_t type; };
template<> struct _impl_IntOfGivenSize<4>{ typedef int32_t type; };
template<> struct _impl_IntOfGivenSize<8>{ typedef int64_t type; };
template<> struct _impl_IntOfGivenSize<16>{ typedef int64_t type; };

template<size_t size> struct _impl_UnsOfGivenSize;
template<> struct _impl_UnsOfGivenSize<1>{ typedef uint8_t type; };
template<> struct _impl_UnsOfGivenSize<2>{ typedef uint16_t type; };
template<> struct _impl_UnsOfGivenSize<4>{ typedef uint32_t type; };
template<> struct _impl_UnsOfGivenSize<8>{ typedef uint64_t type; };
template<> struct _impl_UnsOfGivenSize<16>{ typedef uint64_t type; };

template<size_t size> using IntOfGivenSize = typename _impl_IntOfGivenSize<size>::type;
template<size_t size> using UnsOfGivenSize = typename _impl_UnsOfGivenSize<size>::type;



// OVERLOADS FOR OPERATORS
#define _impl_BINARY_OP_MACRO(op) { \
	constexpr decltype(auto) operator ()(TL lhs, TR rhs) const noexcept{ return lhs op rhs; } \
};

template<class TL, class TR = TL> struct Op_Plus         _impl_BINARY_OP_MACRO(+)
template<class TL, class TR = TL> struct Op_Minus        _impl_BINARY_OP_MACRO(-)
template<class TL, class TR = TL> struct Op_Multiply     _impl_BINARY_OP_MACRO(*)
template<class TL, class TR = TL> struct Op_Divide       _impl_BINARY_OP_MACRO(/)
template<class TL, class TR = TL> struct Op_Modulo       _impl_BINARY_OP_MACRO(%)

template<class TL, class TR = TL> struct Op_Lesser       _impl_BINARY_OP_MACRO(<)
template<class TL, class TR = TL> struct Op_Greater      _impl_BINARY_OP_MACRO(>)
template<class TL, class TR = TL> struct Op_LesserEqual  _impl_BINARY_OP_MACRO(<=)
template<class TL, class TR = TL> struct Op_GreaterEqual _impl_BINARY_OP_MACRO(>=)
template<class TL, class TR = TL> struct Op_Equal        _impl_BINARY_OP_MACRO(==)
template<class TL, class TR = TL> struct Op_NotEqual     _impl_BINARY_OP_MACRO(!=)
template<class TL, class TR = TL> struct Op_LogicOr      _impl_BINARY_OP_MACRO(||)
template<class TL, class TR = TL> struct Op_LogicAnd     _impl_BINARY_OP_MACRO(&&)

template<class TL, class TR = TL> struct Op_BitOr        _impl_BINARY_OP_MACRO(|)
template<class TL, class TR = TL> struct Op_BitAnd       _impl_BINARY_OP_MACRO(&)
template<class TL, class TR = TL> struct Op_BitXor       _impl_BINARY_OP_MACRO(^)
template<class TL, class TR = TL> struct Op_LeftShift    _impl_BINARY_OP_MACRO(<<)
template<class TL, class TR = TL> struct Op_RightShift   _impl_BINARY_OP_MACRO(>>)

#undef _impl_BINARY_OP_MACRO
#define _impl_UNARY_OP_MACRO(op) { \
	constexpr decltype(auto) operator ()(T arg) const noexcept{ return op arg; } \
};

template<class T> struct Op_UnaryPlus   _impl_UNARY_OP_MACRO(+)
template<class T> struct Op_UnaryMinus  _impl_UNARY_OP_MACRO(-)
template<class T> struct Op_LogicNot    _impl_UNARY_OP_MACRO(!)
template<class T> struct Op_BitNot      _impl_UNARY_OP_MACRO(~)
template<class T> struct Op_Dereference _impl_UNARY_OP_MACRO(*)

#undef _impl_UNARY_OP_MACRO




// DEFAULT INITIALIZATIONS AND DEINITIALIZATIONS
template<class T> static void init(T &) noexcept{}
template<class T> static void deinit(T &) noexcept{}



// GENERAL TYPE TARITS
template<class T> constexpr bool needs_init = false;
template<class T> constexpr bool needs_deinit = false;



// COMMON OPERATIONS FOR ARRAYS
template<class T, size_t N> static constexpr size_t len(const T (&)[N]) noexcept{ return N; }
template<class T, size_t N> static constexpr T *beg(T (&arr)[N]) noexcept{ return (T *)arr; }
template<class T, size_t N> static constexpr T *end(T (&arr)[N]) noexcept{ return (T *)arr + N; }
template<class T, size_t N> static constexpr bool needs_init<T (&)[N]> = needs_init<T>;
template<class T, size_t N> static constexpr bool needs_deinit<T (&)[N]> = needs_deinit<T>;


template<class T, size_t N> static
void init(T (&arr)[N]) noexcept{ for (T *I=arr; I!=arr+N; ++I) init(*I); }

template<class T, size_t N> static
void deinit(T (&arr)[N]) noexcept{ for (T *I=arr; I!=arr+N; ++I) deinit(*I); }


template<class TD, class TS> static
void copy(TD &dest, const TS &src) noexcept{
	if constexpr (std::is_array_v<TS>)
		for (size_t i=0; i!=len(src); ++i) copy(dest[i], src[i]);
	else
		dest = src;
}



// GENERIC MATH CONSTANTS
template<class T> static constexpr T Null = T{};
template<class T> static constexpr T Unit = (T)1;



// COMMON SIMPLE GENERIC FUNCTIONS
template<class T> static constexpr
T &min(T &x, T &y) noexcept{ return x>y ? y : x; }

template<class T> static constexpr
const T &min(const T &x, const T &y) noexcept{ return x>y ? y : x; }

template<class T> static constexpr
T &max(T &x, T &y) noexcept{ return x<y ? y : x; }

template<class T> static constexpr
const T &max(const T &x, const T &y) noexcept{ return x<y ? y : x; }



// SIMPLE SPAN CLASS
template<class T>
struct Span{
	constexpr
	T &operator[](size_t index) noexcept{ return *(ptr + index); }
	
	constexpr
	const T &operator[](size_t index) const noexcept{ return *(ptr + index); }

	constexpr
	operator Span<uint8_t>() const noexcept{
		return Span<uint8_t>{(uint8_t *)ptr, (size*sizeof(T))};
	}
	
	constexpr
	operator Span<std::add_const_t<T>>() const noexcept{
		return Span<std::add_const_t<T>>{
			(std::add_pointer_t<std::add_const_t<T>>)ptr, 
			size
		};
	}

	typedef T ValueType;

	T *ptr;
	size_t size;
};


template<class T> static constexpr
size_t len(Span<T> range) noexcept{ return range.size; }

template<class T> static constexpr  
T *begin(Span<T> range) noexcept{ return range.ptr; }

template<class T> static constexpr  
T *beg(Span<T> range) noexcept{ return range.ptr; }

template<class T> static constexpr
T *end(Span<T> range) noexcept{ return range.ptr + range.size; }

template<class T> static
void resize(Span<T> &range, size_t size) noexcept{ range.size = size; }

template<class T> static
void shrink_back(Span<T> &range, size_t size) noexcept{ range.size -= size; }

template<class T> static
void shrink_front(Span<T> &range, size_t size) noexcept{ range.ptr += size; }


template<class T> static
void pop(Span<T> &range) noexcept{ --range.size; }

template<class T> static
void pop_front(Span<T> &range) noexcept{ ++range.ptr; }

template<class T> static
T &pop_val(Span<T> &range) noexcept{
	return *(range.ptr + --range.size);
}

template<class T> static
T &pop_front_val(Span<T> &range) noexcept{
	T *res = range.ptr;
	++range.ptr;
	return *res;
}


template<class Cont> static constexpr
auto slice(Cont &cont) noexcept{
	return Span<std::remove_reference_t<decltype(cont[0])>>{beg(cont), len(cont)};
}

template<class Cont> static constexpr
auto slice(const Cont &cont) noexcept{
	return Span<std::remove_reference_t<decltype(cont[0])>>{beg(cont), len(cont)};
}

template<class Cont> static constexpr
auto slice(Cont &cont, size_t from, size_t to) noexcept{
	return Span<std::remove_reference_t<decltype(cont[0])>>{beg(cont)+from, to-from};
}

template<class Cont> static constexpr
auto slice(const Cont &cont, size_t from, size_t to) noexcept{
	return Span<std::remove_reference_t<decltype(cont[0])>>{beg(cont)+from, to-from};
}

template<class T> static constexpr
auto slice(T *first, T *last) noexcept{ return Span<T>{first, last-first}; }

template<class T> static constexpr
auto slice(const T *first, const T *last) noexcept{ return Span<const T>{first, last-first}; }


template<class TL, class TR> static constexpr
bool operator ==(Span<TL> lhs, Span<TR> rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const TL *sent = lhs.ptr + lhs.size;
	const TR *J = rhs.ptr;
	for (const TL *I=lhs.ptr; I!=sent; ++I, ++J) if (*I != *J) return false;
	return true;
}

template<class TL, class TR> static constexpr
bool operator !=(Span<TL> lhs, Span<TR> rhs) noexcept{ return !(lhs == rhs); }






// MEMORY AND POINTERS
using Memblock = Span<uint8_t>;


uint8_t *alignptr(void *ptr, size_t alignment) noexcept{
	return (uint8_t *)(((uintptr_t)ptr - 1u + alignment) & -alignment);
}

const void *alignptr(const void *ptr, size_t alignment) noexcept{
	return (const uint8_t *)(((uintptr_t)ptr - 1u + alignment) & -alignment);
}


template<class T> static T &deref(T &arg) noexcept{ return arg; }
template<class T> static T &deref(T *arg) noexcept{ return *arg; }
template<class T> static const T &deref(const T &arg) noexcept{ return arg; }
template<class T> static const T &deref(const T *arg) noexcept{ return *arg; }




// SIMPLE LOW LEVEL DYNAMIC ARRAY CLASS FOR MANUAL MEMORY ALLOCATION
template<class T>
struct Array{
	T &operator [](size_t i) noexcept{ return *((T *)this->ptr + i); }
	const T &operator [](size_t i) const noexcept{ return *((const T *)this->ptr + i); }

	typedef T ValueType;

	uint8_t *ptr = nullptr;
	uint32_t size = 0;
	uint32_t cap = 0;
};

template<class T> static constexpr
size_t len(const Array<T> &arr) noexcept{ return arr.size; }

template<class T> static constexpr
T *begin(const Array<T> &arr) noexcept{ return (T *)arr.ptr; }

template<class T> static constexpr
T *beg(const Array<T> &arr) noexcept{ return (T *)arr.ptr; }

template<class T> static constexpr
T *end(const Array<T> &arr) noexcept{ return (T *)arr.ptr + arr.size; }

template<class T> static constexpr
size_t cap(const Array<T> &arr) noexcept{ return arr.cap / sizeof(T); }

template<class T> static constexpr
bool is_empty(const Array<T> &arr) noexcept{ return arr.size == 0; }

template<class T> static constexpr
bool operator ==(const Array<T> &lhs, const Array<T> &rhs) noexcept{
	if (lhs.size != rhs.size) return false;
	const T *sent = (const T *)lhs.ptr + lhs.size;
	for (const T *I=(const T *)lhs.ptr, *J=(const T *)rhs.ptr; I!=sent; ++I, ++J)
		if (*I != *J) return false;
	return true;
}

template<class T> static constexpr
bool operator !=(const Array<T> &lhs, const Array<T> &rhs) noexcept{
	return !(lhs == rhs);
}

template<class T> static constexpr bool needs_deinit<Array<T>> = true;

template<class T> static
void deinit(Array<T> &arr) noexcept{
	if constexpr (needs_deinit<T>)
		for (size_t i=0; i!=arr.size; ++i) deinit(*((T *)arr.ptr+1));
}

template<class T, class A> static
void deinit(Array<T> &arr, A &allocator) noexcept{
	if constexpr (needs_deinit<T>)
		for (size_t i=0; i!=arr.size; ++i) deinit(*((T *)arr.ptr+1));
	free(allocator, Memblock{arr.ptr, arr.cap});
}

template<class T, class A> static
bool resize(Array<T> &arr, A &allocator, size_t size) noexcept{
	if (arr.size < size){
		size_t bytes = size * sizeof(T);
		if (arr.cap < bytes){
			Memblock blk;
			if constexpr (A::Alignment)
				blk = realloc(allocator, Memblock{arr.ptr, arr.cap}, bytes);
			else
				blk = realloc(allocator, Memblock{arr.ptr, arr.cap}, bytes, alignof(T));
			if (blk.ptr == nullptr) return true;
			arr.ptr = blk.ptr;	
			arr.cap = blk.size;	
		}

		if constexpr (needs_init<T>)
			for (T *I=(T *)arr.ptr+arr.size; I!=(T *)arr.ptr+size; ++I) init(*I);
	} else{
		if constexpr (needs_deinit<T>)
			for (T *I=(T *)arr.ptr+size; I!=(T *)arr.ptr+arr.size; ++I) deinit(*I);
	}
	
	arr.size = size;
	return false;
}

template<class T> static
void shrink_back(Array<T> &arr, size_t amount) noexcept{
	if constexpr (needs_deinit<T>)
		for (T *I=(T *)arr.ptr+arr.size-amount; I!=(T *)arr.ptr+arr.size; ++I) deinit(*I);
	
	arr.size -= amount;
}

template<class T, class A> static
bool expand_back(Array<T> &arr, size_t amount, A &allocator) noexcept{
	size_t size = arr.size + amount;
	size_t bytes = size * sizeof(T);
	if (arr.cap < bytes){
		Memblock blk;
		blk = realloc(allocator, Memblock{arr.ptr, arr.cap}, bytes);
		if (blk.ptr == nullptr) return true;
		arr.ptr = blk.ptr;
		arr.cap = blk.size;
	}

	if constexpr (needs_init<T>)
		for (T *I=(T *)arr.ptr+arr.size; I!=(T *)arr.ptr+size; ++I) init(*I);
	
	arr.size = size;
	return false;
}


template<class T, class A> static
bool push(Array<T> &arr, A &allocator) noexcept{
	if (arr.size == arr.cap/sizeof(T)){
		Memblock blk;
		blk = realloc(
			allocator, Memblock{arr.ptr, arr.cap},
			arr.size ? 2*arr.size*sizeof(T) : sizeof(T)<64 ? (64/sizeof(T))*sizeof(T) : sizeof(T)
		);
		if (blk.ptr == nullptr) return true;
		arr.ptr = blk.ptr;
		arr.cap = blk.size;
	}
	if constexpr (needs_init<T>) init(*((T *)arr.ptr+arr.size));

	arr.size += 1;
	return false;
}

template<class T, class TV, class A> static
bool push_value(Array<T> &arr, const TV &value, A &allocator) noexcept{
	bool err = push(arr, allocator);
	if (!err) copy(*((T *)arr.ptr+arr.size-1), value);
	return err;
}

template<class T, class TV, class A> static
bool push_range(Array<T> &arr, Span<TV> range, A &allocator) noexcept{
	size_t size = arr.size + range.size;
	if (arr.cap < size*sizeof(T)){
		Memblock blk;

		blk = realloc(allocator, Memblock{arr.ptr, arr.cap}, size);
		if (blk.ptr == nullptr) return true;
		arr.ptr = blk.ptr;
		arr.cap = blk.size;
	}

	T *J = (T *)arr.ptr + arr.size;
	for (const TV *I=range.ptr; I!=range.ptr+range.size; ++I, ++J){
		if constexpr (needs_init<T>) init(*J);
		*J = *I;
	}
	arr.size = size;
	return false;
}

template<class T> static
void pop(Array<T> &arr) noexcept{ --arr.size; }

template<class T> static
T &&pop_value(Array<T> &arr) noexcept{ return (T &&)((T *)arr.ptr)[--arr.size]; }






// SIMPLE TUPLE TYPE
template <class T, class... TR>
struct Tuple{
	typedef bool TupleFlag;
	T head;
	Tuple<TR ...> tail;
};

template <class T>
struct Tuple<T>{
	typedef bool TupleFlag;
	T head;
};

template<size_t index, class... TR> constexpr
const auto &get(const Tuple<TR ...> &tuple) noexcept{
	if constexpr (index == 0)
		return tuple.head;
	else
		return get<index-1>(tuple.tail);
}
template<size_t index, class... TR> constexpr
auto &get(Tuple<TR ...> &tuple) noexcept{
	if constexpr (index == 0)
		return tuple.head;
	else
		return get<index-1>(tuple.tail);
}

template<class... TR> constexpr
size_t len(const Tuple<TR ...> &tuple) noexcept{ return sizeof...(TR); }

template<class T>
constexpr bool is_tuple = false;
template<class... TR>
constexpr bool is_tuple<Tuple<TR ...>> = true;

template<class T, class... TR>
constexpr bool needs_init<Tuple<T, TR ...>> = needs_init<T> || needs_init<Tuple<TR ...>>;
template<class T>
constexpr bool needs_init<Tuple<T>> = needs_init<T>;

template<class T, class... TR>
constexpr bool needs_deinit<Tuple<T, TR ...>> = needs_deinit<T> || needs_deinit<Tuple<TR ...>>;
template<class T>
constexpr bool needs_deinit<Tuple<T>> = needs_deinit<T>;

template<class T, class... TR>
void init(Tuple<TR ...> &tuple) noexcept{
	init(tuple.head);
	if constexpr (len(tuple) != 1) init(tuple.tail);
}

template<class T, class... TR>
void deinit(Tuple<TR ...> &tuple) noexcept{
	deinit(tuple.head);
	if constexpr (len(tuple) != 1) deinit(tuple.tail);
}





// MOVING THE VALUES
template<class T> static
void swap(T &lhs, T &rhs) noexcept{
	T temp = lhs;
	lhs = rhs;
	rhs = temp;
}

template<class T> static
void iswap(T *lhs, T *rhs) noexcept{
	T temp = *lhs;
	*lhs = *rhs;
	*rhs = temp;
}




// GENERAL MATH
template<class T> static constexpr
T abs(T x) noexcept{ return (x < (T)0) ? -x : x; }

template<class T> static constexpr
T sign(T x) noexcept{
	if (x == (T)0) return (T)0;
	return x < (T)0 ? (T)-1 : (T)1;
}



// INTEGER MATH
static constexpr
size_t factorial(size_t n) noexcept{
	size_t result = n;
	while (--n) result *= n;
	return result;
}

template<class T> static constexpr
std::enable_if_t<std::is_integral_v<T>, T> int_sqrt(T x) noexcept{
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

template<class T> static constexpr
T gcd(T m, T n) noexcept{       // copied from libstdc++
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

template<class T> static constexpr
T lcm(T m, T n) noexcept{ // copied from libstdc++
	return m && n ? (m / gcd(m, n))*n : 0;
}



// FLOATING POINT MATH
static constexpr
int32_t get_mantissa(float x) noexcept{
	if (x < 0.f)
		return -((1<<23) | (std::bit_cast<int32_t>(x) & 0x007fffff));
	return (1<<23) | (std::bit_cast<int32_t>(x) & 0x007fffff);
}

static constexpr
int32_t get_exponent(float x) noexcept{
	return ((std::bit_cast<int32_t>(x) & 0x7fffffff) >> 23) - 0x7f;
}

static constexpr
int64_t get_mantissa(double x) noexcept{
	if (x < 0.0)
		return -((1ll<<52) | (std::bit_cast<int64_t>(x) & 0x000fffffffffffff));
	return (1ll<<52) | (std::bit_cast<int64_t>(x) & 0x000fffffffffffff);
}

static constexpr
int64_t get_exponent(double x) noexcept{
	return ((std::bit_cast<int64_t>(x) & 0x7fffffffffffffff) >> 52) - 0x3ff;
}



// SIMPLE TYPE ERASING DATA TYPE FOR PASSING GENERAL FUNCTIONS
template<class Signature> struct Delegate;

template<class Res, class... Args>
struct Delegate<Res(Args...)>{
	template<class Proc>
	Delegate(const Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
	}
	
	template<class Proc>
	Delegate(Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
	}

	template<class Proc>
	Delegate(const Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = (Proc *)&proc;
	}
	
	template<class Proc>
	Delegate(Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
	}
	
	template<class Proc>
	Delegate &operator =(const Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
		return *this;
	}
	
	template<class Proc>
	Delegate &operator =(Proc *proc) noexcept{
		static_assert(std::is_function_v<Proc>);
		
		procedure = (void *)&proc;
		object = nullptr;
		return *this;
	}

	template<class Proc>
	Delegate &operator =(const Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = (Proc *)&proc;
		return *this;
	}
	
	template<class Proc>
	Delegate &operator =(Proc &proc) noexcept{
		static_assert(std::is_invocable_v<Proc>);

		procedure = (void *)(Res (Proc::*)(Args...))&Proc::operator();
		object = &proc;
		return *this;
	}
	

	Delegate() noexcept = default;
	Delegate(const Delegate &) noexcept = default;
	Delegate(Delegate &) noexcept = default;
	Delegate &operator =(const Delegate &) noexcept = default;
	Delegate &operator =(Delegate &) noexcept = default;
	Delegate(void *proc, const void *obj) noexcept : procedure{proc}, object{(void *)obj} {}
	Delegate(void *proc, void *obj) noexcept : procedure{proc}, object{obj} {}


	Res operator ()(Args... args) noexcept{
		return object ? (
			((Res (*)(void *, Args...))procedure)(object, args...)
		) : (
			((Res (*)(Args...))procedure)(args...)
		);
	}


	typedef Res ResultType;

	const void *procedure;
	void *object;
};



// SIMPLE RANDOM NUMBER GENERATOR
struct Rand32{
	constexpr uint32_t operator ()() noexcept{
		this->seed += 0xe120fc15;
		uint64_t temp = (uint64_t)seed * 0x4a39b70d;
		temp = (uint64_t)((temp >> 32) ^ temp) * 0x12fad5c9;
		return (temp >> 32) ^ temp;
	}

	typedef uint32_t ValueType;

	uint32_t seed;
};

static constexpr
uint32_t min_val(Rand32) noexcept{ return 0; }

static constexpr
uint32_t max_val(Rand32) noexcept{ return UINT32_MAX; }



// FAST FLOATING POINT MATH
static constexpr
float qlog(float x) noexcept{
	constexpr float scaleDown = 1.f/(float)0x00800000;
	return (float)(std::bit_cast<uint32_t>(x) - std::bit_cast<uint32_t>(1.f))*scaleDown;
}

static constexpr
float qexp(float x) noexcept{
	constexpr float scaleUp = 0x00800000;
	return std::bit_cast<float>((uint32_t)(x*scaleUp) + std::bit_cast<uint32_t>(1.f));
}

static constexpr
float qsqrt(float x) noexcept{
	uint32_t i = std::bit_cast<uint32_t>(x);
	i -= 1<<23;
	i >>= 1;
	i += 1<<29;
	float f = std::bit_cast<float>(i);
	return (f + x/f)*0.5f;
}

static constexpr
float qinv_sqrt(float x) noexcept{
	float f = std::bit_cast<float>((uint32_t)0x5f3759df - (std::bit_cast<uint32_t>(x)>>1));
	return f * (1.5f - 0.5f*x*f*f);
}

