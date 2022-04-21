#include "SPL/Utils.hpp"
#include <variant>
#include <iostream>


template<class... Args_t>
struct Allof_t;

template<class Arg0_t, class... Args_t>
struct Allof_t<Arg0_t, Args_t ...>{
	const Allof_t<Args_t ...> rest;
	const Arg0_t arg;

	constexpr Allof_t(const Arg0_t &t0, const Args_t &...tt) : rest{tt...}, arg{t0} {}
	Allof_t(const Allof_t &) = default;

	constexpr bool operator ==(const Arg0_t &lhs) const{ return arg == lhs && rest == lhs; }
};

template<class Arg0_t>
struct Allof_t<Arg0_t>{
	const Arg0_t arg;

	constexpr Allof_t(const Arg0_t &t0) : arg{t0} {}
	Allof_t(const Allof_t &) = default;


	constexpr bool operator ==(const Arg0_t &lhs) const{ return arg == lhs; }
};

template<class... Args_t>
constexpr auto allof(const Args_t ...args){ return Allof_t<Args_t ...>{args...}; }




template<class... Args_t>
struct Anyof_t;

template<class Arg0_t, class... Args_t>
struct Anyof_t<Arg0_t, Args_t ...>{
	const Anyof_t<Args_t ...> rest;
	const Arg0_t arg;

	constexpr Anyof_t(const Arg0_t &t0, const Args_t &...tt) : rest{tt...}, arg{t0} {}
	Anyof_t(const Anyof_t &) = default;

	constexpr bool operator ==(const Arg0_t &rhs) const{ return arg == rhs || rest == rhs; }
};

template<class Arg0_t>
struct Anyof_t<Arg0_t>{
	const Arg0_t arg;

	constexpr Anyof_t(const Arg0_t &t0) : arg{t0} {}
	Anyof_t(const Anyof_t &) = default;

	constexpr bool operator ==(const Arg0_t &rhs) const{ return arg == rhs; }
};




template<class... Args_t>
constexpr auto anyof(const Args_t &...args){ return Anyof_t<Args_t ...>{args...}; }

template<class... Args_t>
constexpr auto getSum(const Args_t &...args){ return (args + ...); }




template<class T>
const T &printVal(const T &x){ std::cout << x << '\n'; return x; }




template<uint32_t index, class Arg0_t, class... Args_t>
constexpr const auto &getArg(const Arg0_t &arg0, const Args_t &...args){
	if constexpr (index)
		return getArg<index-1>(args...);
	else
		return arg0;
}
template<uint32_t index, class Arg0_t>
constexpr const auto &getArg(const Arg0_t &arg0){ return arg0; }

template<uint32_t index, class Arg0_t, class... Args_t>
constexpr auto &getArg(Arg0_t &arg0, Args_t &...args){
	if constexpr (index)
		return getArg<index-1>(args...);
	else
		return arg0;
}
template<uint32_t index, class Arg0_t>
constexpr auto &getArg(Arg0_t &arg0){ return arg0; }




template<uint32_t index, class... Args_t>
void printArg(Args_t ...args){
	std::cout << getArg<index>(args...) << '\n';
}




int main(){

	// printf(allof(1, 1, 1, 1) == 1 ? "true\n" : "false\n");


	// auto sum = getSum(
	// 	printVal(1),
	// 	printVal(2),
	// 	printVal(3),
	// 	printVal(4)
	// );

	// printVal(sum);


	allof(1, 2, 3, 4, 5) == 1;
	anyof(1, 2, 3, 4, 5) == 1;

	printArg<4>(1, 2, 3, 4, 5, 6);

	constexpr auto x = getArg<2>(0, "dupsko", 2, 3, 4, 5);

	return 0;
}