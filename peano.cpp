#include <iostream>
#include <cassert>
#include <chrono>

#define test_value 27 
#define num_trials1 10000000
// 27 -> 111

template
<
  template<class...> class T,
  typename... Args
>
struct lazy_template {
  using instantiated = T<Args...>;
};

template<class T>
struct NOP {
	using type = T;
};

struct Error{};
struct Negative: Error {};
struct Indeterminate: Error {};
struct Undefined: Error {};

struct Peano {};
struct Zero: Peano {};
template<class T>
struct Succ: Peano {};

template<class T1, class T2>
struct Add;
template<class T>
struct Add<Zero, T> {
	using type = T;
};
#pragma GCC diagnostic ignored "-Wstack-exhausted"
template<class T1, class T2>
struct Add<Succ<T1>, T2> {
	using type = Succ<typename Add<T1, T2>::type>;
};

template <class T1, class T2>
struct Sub {
	using type = Undefined;
};

template<>
struct Sub<Zero, Zero> {
	using type = Zero;
};
template<class T>
struct Sub<T, Zero> {
	using type = T;
};
template<class T>
struct Sub<Zero, T> {
	using type = Negative;
};
template<class T1, class T2>
struct Sub<Succ<T1>, Succ<T2>> {
	using type = typename Sub<T1, T2>::type;
};

template<int i>
struct pretty_num {
 	using type = Succ<typename pretty_num<i-1>::type>;
};
template<>
struct pretty_num<0> {
 	using type = Zero;
};

template<class T>
struct accum {};
template<>
struct accum<Zero> {
	int inside = 0;
};
template<class T>
struct accum<Succ<T>> {
	int inside = 1 + accum<T>().inside;
};

template<class T1, class T2>
struct Mult;
template<>
struct Mult<Zero, Zero> {
	using type = Zero;
};
template<class T>
struct Mult<Zero, T> {
	using type = Zero;
};
// template<class T>
// struct Mult<T, Zero> {
// 	using type = Zero;
// };
template<class T1, class T2>
struct Mult<Succ<T1>, T2> {
	using type = typename Add<typename Mult<T1, T2>::type,
	                                   T2>::type;	
};

template<class T1, class T2>
struct Div
{
	using sub = typename Sub<T1, T2>::type;
	using temp_div = typename Div<sub, T2>::type;
	using type = Succ<temp_div>;
};
template<> 
struct Div<Zero, Zero>
{
	using type = Indeterminate;	
};
template<class T>
struct Div<Zero, T>
{
	using type = Zero;	
};
template<class T>
struct Div<T, Zero>
{
	using type = Undefined;	
};
template<class T>
struct Div<T, Succ<Zero>> {
	using type = T;
};
template<class T>
struct Div<Negative, T> {
	using type = Undefined;
};

template<class T1, class T2>
struct GT {};
template<class T>
struct GT<Zero, T>
{
	using type = std::false_type;
};
template<class T1, class T2>
struct GT<Succ<T1>, Succ<T2>>
{
	using type = typename GT<T1, T2>::type;
};
template<class T>
struct GT<Succ<T>, Zero>
{
	using type = std::true_type;
};

template<class T1, class T2>
struct EQ {};
template<class T1, class T2>
struct EQ<Succ<T1>, Succ<T2>>
{
	using type = typename EQ<T1, T2>::type;
};
template<class T>
struct EQ<Zero, Succ<T>>
{
	using type = std::false_type;
};
template<class T>
struct EQ<Succ<T>, Zero>
{
	using type = std::false_type;
};
template<>
struct EQ<Zero, Zero>
{
	using type = std::true_type;
};

template <class T>
struct NOT{};
template<>
struct NOT<std::true_type> {
	using type = std::false_type;
};
template<>
struct NOT<std::false_type> {
	using type = std::true_type;
};

template <class T1, class T2>
struct AND{};
template<>
struct AND<std::true_type, std::true_type> {
	using type = std::true_type;
};
template<>
struct AND<std::true_type, std::false_type> {
	using type = std::false_type;
};
template<>
struct AND<std::false_type, std::true_type> {
	using type = std::false_type;
};
template<>
struct AND<std::false_type, std::false_type> {
	using type = std::false_type;
};

// !(!a && !b) == (a || b)
template <class T1, class T2>
struct OR{
	using type = typename NOT<typename AND<typename NOT<T1>::type, typename NOT<T2>::type>::type>::type;
};

template <class T1, class T2>
struct GEQ {
	using eq_sub =  typename EQ<T1, T2>::type;
	using gt_sub =  typename GT<T1, T2>::type;
	using type = typename OR<eq_sub, gt_sub>::type;
};

// TODO
// template <bool Condition, class T1, class T2>
// struct lazy_conditional {
// 	using lazy_t1 = lazy_template<NOP, T1>;
// 	using lazy_t2 = lazy_template<NOP, T2>;
// 	using type = typename std::conditional<Condition, typename lazy_t1::instantiated, typename lazy_t2::instantiated>::type;
// };

// Mod requires better errors or better sense of GT and LT
template<class T1, class T2>
struct Mod
{
	using temp_sub = typename Sub<T1,T2>::type;
	using lazy_recurse = lazy_template<Mod, temp_sub, T2>;
	using lazy_t1 = lazy_template<NOP, T1>;
	using type = typename std::conditional<std::is_same<temp_sub, Negative>::value,
	                                       typename lazy_t1::instantiated,
	                                       typename lazy_recurse::instantiated>::type::type;
};
template<> 
struct Mod<Zero, Zero>
{
	using type = Indeterminate;	
};
template<class T>
struct Mod<Zero, T>
{
	using type = Zero;
};

template<class ...elements> struct Tuple{};


template <class peano, class ...Ts> struct GetElem {
	template <class U, class ...Us> struct helper {
		using lazy_car = lazy_template<NOP, U>;
		using lazy_cdr = lazy_template<Tuple, Us...>;
	};
	template<class T> struct extract_elem {
		using type = T;
	};
	template<class T> struct extract_elem<Tuple<T>> {
		using type = T;
	};
	using internal = helper<Ts...>;
	using car_or_cdr = typename std::conditional<std::is_same<peano, Zero>::value,
										         typename internal::lazy_car::instantiated,
										         typename internal::lazy_cdr::instantiated>::type::type;
	// using result = typename std::conditional<std::is_same<peano, Zero>::value,
	// 									         car_or_cdr,
	// 									         typename GetElem(typename Sub<peano, Succ<Zero>>, car_or_cdr>>::type::type;
    // Remove wrapper tuple
    using type = typename extract_elem<car_or_cdr>::type;
};

template<int i, class ...args>
struct Board {
	using size = typename pretty_num<i>::type;
	using elems = Tuple<args...>;
};

template <class T>
struct isEven {
	using type = typename EQ<typename Mod<T, typename pretty_num<2>::type>::type,Zero>::type;
};

template <class T>
struct isTwo {
	using type = typename EQ<T, typename pretty_num<2>::type>::type;
};

template <class T>
struct next {
	using lazy_even = lazy_template<Div, T, typename pretty_num<2>::type>;
	using lazy_odd = lazy_template<Add, typename Mult<T, typename pretty_num<3>::type>::type, typename pretty_num<1>::type>;
	using type = typename std::conditional<isEven<T>::type::value,
	                               typename lazy_even::instantiated,
	                               typename lazy_odd::instantiated>::type::type;
};
template<>
struct next<Zero> {
	using type = Undefined;
};

template<class T>
struct num_steps {
	using type = typename Add<typename pretty_num<1>::type, typename num_steps<typename next<T>::type>::type>::type;
};
template<>
struct num_steps<Zero> {
	using type = Undefined;
};
template<>
struct num_steps<Succ<Zero>> {
	using type = Zero;
};

// Doing collatz the sane way
int sane_next(int x){
	if (x % 2 == 0)
	{
		return x / 2;
	} else {
		return 3*x + 1;
	}
}
int sane_num_steps(int x) {
	int count = 0;
	while (x > 1) {
		x = sane_next(x);
		count ++;
	}
	return count;
}
int sane_recur_num_steps(int x) {
	if (x == 1) {
		return 0;
	} else {
		return 1 + sane_recur_num_steps(sane_next(x));
	}
}

//timing our code
double time_function(int(*foo)(int), int x) {
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < num_trials1; ++i)
	{
		foo(x);
	}
    std::cout << foo(x) << " : ";
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
	return elapsed_seconds.count();
}

int main()
{
	std::cout << "Test" << std::endl;

    // Error test
	static_assert((std::is_same<Indeterminate, Indeterminate>::value), "Indeterminate == Indeterminate");
	static_assert((std::is_same<Undefined, Undefined>::value), "Undefined == Undefined");
	static_assert((!std::is_same<Undefined, Indeterminate>::value), "Undefined != Indeterminate");
	static_assert((!std::is_same<Indeterminate, Undefined>::value), "Indeterminate != Undefined");

	// Zero
	static_assert((std::is_same<Zero, Zero>::value), "Zero is zero");

	// Addition
	static_assert((std::is_same<Add<Zero, Zero>::type, Zero>::value), "0+0 = 0");
	static_assert((!std::is_same<Add<Zero, Zero>::type, Succ<Zero>>::value), "0+0 != 1");
	static_assert((std::is_same<Add<Succ<Zero>, Zero>::type, Succ<Zero>>::value), "1+0 = 1");
	static_assert((std::is_same<Add<Zero, Succ<Zero>>::type, Succ<Zero>>::value), "0+1 = 1");
	static_assert((std::is_same<Add<Succ<Zero>, Succ<Zero>>::type, Succ<Succ<Zero>>>::value), "1+1 = 2");
	static_assert((std::is_same<Add<Succ<Succ<Zero>>, Succ<Zero>>::type, Succ<Succ<Succ<Zero>>>>::value), "2+1 = 3");

	// Subtraction
	static_assert((std::is_same<Sub<Zero, Zero>::type, Zero>::value), "0-0 = 0");
	static_assert((!std::is_same<Sub<Zero, Zero>::type, Succ<Zero>>::value), "0-0 != 1");
	static_assert((std::is_same<Sub<Succ<Zero>, Zero>::type, Succ<Zero>>::value), "1-0 = 1");
	static_assert((std::is_same<Sub<Zero, Succ<Zero>>::type, Negative>::value), "0-1 = Error");
	static_assert((std::is_same<Sub<Zero, Succ<Succ<Zero>>>::type, Negative>::value), "0-2 = Error");
	static_assert((std::is_same<Sub<Succ<Zero>, Succ<Zero>>::type, Zero>::value), "1-1 = 0");
	static_assert((std::is_same<Sub<Succ<Succ<Succ<Zero>>>, Succ<Succ<Zero>>>::type, Succ<Zero>>::value), "3-2 = 1");
	static_assert((std::is_same<Sub<pretty_num<3>::type, pretty_num<2>::type>::type, pretty_num<1>::type>::value), "3-2 = 1");

	// Pretty num
	static_assert((std::is_same<pretty_num<0>::type, Zero>::value), "0 is Zero");
	static_assert((std::is_same<pretty_num<1>::type, Succ<Zero>>::value), "1 is Succ(Zero)");
	static_assert((std::is_same<Add<pretty_num<1>::type,pretty_num<1>::type>::type, pretty_num<2>::type>::value), "1+1 is 2");

	// Mult
	static_assert((std::is_same<Mult<Zero, Zero>::type, Zero>::value), "0*0 = 0");
	static_assert((!std::is_same<Mult<Zero, Zero>::type, Succ<Zero>>::value), "0*0 != 1");
	static_assert((std::is_same<Mult<Succ<Zero>, Zero>::type, Zero>::value), "1*0 = 0");
	static_assert((std::is_same<Mult<Zero, Succ<Zero>>::type, Zero>::value), "0*1 = 0");
	static_assert((std::is_same<Mult<Succ<Zero>, Succ<Zero>>::type, Succ<Zero>>::value), "1*1 = 1");
	static_assert((std::is_same<Mult<Succ<Succ<Succ<Zero>>>, Succ<Succ<Zero>>>::type, pretty_num<6>::type>::value), "3*2 = 6");
	static_assert((std::is_same<Mult<pretty_num<17>::type, pretty_num<3>::type>::type, pretty_num<51>::type>::value), "17*3 = 51");

	// Div
	static_assert((std::is_same<Div<Zero, Zero>::type, Indeterminate>::value), "0/0 = Indeterminate");
	static_assert((!std::is_same<Div<Zero, Zero>::type, Succ<Zero>>::value), "0/0 != 1");
	static_assert((std::is_same<Div<Succ<Zero>, Zero>::type, Undefined>::value), "1/0 = Undefined");
	static_assert((std::is_same<Div<Succ<Zero>, Succ<Zero>>::type, Succ<Zero>>::value), "1/1 = 1");
	static_assert((std::is_same<Div<pretty_num<10>::type, pretty_num<2>::type>::type, pretty_num<5>::type>::value), "10 / 2 = 5");
	// TODO make errors generic?
	// static_assert((std::is_same<Div<pretty_num<2>::type, pretty_num<10>::type>::type, Undefined>::value), "2 / 10 = Undefined");

	// Comparison
	static_assert((std::is_same<GT<pretty_num<0>::type, pretty_num<0>::type>::type, std::false_type>::value), "0>0 false");
	static_assert((std::is_same<GT<pretty_num<1>::type, pretty_num<0>::type>::type, std::true_type>::value), "1>0 true");
	static_assert((std::is_same<GT<pretty_num<0>::type, pretty_num<1>::type>::type, std::false_type>::value), "0>1 false");
	static_assert((std::is_same<std::true_type, std::true_type>::value), "true is true");
	static_assert((std::is_same<EQ<pretty_num<0>::type, pretty_num<0>::type>::type, std::true_type>::value), "0==0 true");
	static_assert((std::is_same<EQ<pretty_num<1>::type, pretty_num<1>::type>::type, std::true_type>::value), "1==1 true");
	static_assert((std::is_same<EQ<pretty_num<1>::type, pretty_num<2>::type>::type, std::false_type>::value), "1==2 false");
	static_assert((std::is_same<EQ<pretty_num<2>::type, pretty_num<1>::type>::type, std::false_type>::value), "2==1 false");
	static_assert((EQ<pretty_num<0>::type, pretty_num<0>::type>::type::value), "0==0");
	static_assert((std::is_same<GEQ<pretty_num<0>::type, pretty_num<0>::type>::type, std::true_type>::value), "0>=0 true");
	static_assert((std::is_same<GEQ<pretty_num<1>::type, pretty_num<0>::type>::type, std::true_type>::value), "1>=0 true");
	static_assert((std::is_same<GEQ<pretty_num<0>::type, pretty_num<1>::type>::type, std::false_type>::value), "0>=1 false");

	static_assert((std::is_same<Sub<Zero, Succ<Zero>>::type, Negative>::value), "(0 - 1) == Negative");
	static_assert((std::is_same<std::conditional<true, std::true_type, std::false_type>::type, std::true_type>::value), "(0 - 1) == Negative");

	// std::conditional syntax test
	static_assert((std::is_same<std::conditional<std::is_same<Sub<Zero, Succ<Succ<Succ<Zero>>>>::type, Negative>::value, std::true_type, std::false_type>::type, std::true_type>::value), "(0 - 1) == Negative");

	// // Mod
	static_assert((std::is_same<Mod<pretty_num<2>::type, pretty_num<10>::type>::type, pretty_num<2>::type >::value), "2 % 10 = 2");
	static_assert((std::is_same<Sub<pretty_num<1>::type, pretty_num<2>::type>::type, Negative >::value), "1 - 3 = Negative");
	static_assert((std::is_same<Mod<pretty_num<1>::type, pretty_num<3>::type>::type, pretty_num<1>::type>::value), "1 % 3 = 1");
	static_assert((std::is_same<Mod<pretty_num<2>::type, pretty_num<3>::type>::type, pretty_num<2>::type>::value), "2 % 3 = 2");
	static_assert((std::is_same<Mod<pretty_num<4>::type, pretty_num<3>::type>::type, Mod<pretty_num<1>::type, pretty_num<3>::type>::type>::value), "4 % 3 = 1 % 3");
	static_assert((std::is_same<Mod<pretty_num<4>::type, pretty_num<3>::type>::type, pretty_num<1>::type>::value), "4 % 3 = 1");

	static_assert((!std::is_same<Tuple<std::true_type>, Tuple<std::false_type>>::value), "{true} != {false}");
	static_assert((!std::is_same<Tuple<std::true_type, std::true_type>, Tuple<std::true_type, std::false_type>>::value), "{true, true} != {true, false}");
	static_assert((std::is_same<GetElem<pretty_num<0>::type, Tuple<std::true_type>>::type, std::true_type>::value), "{true}[0] == true");
	// static_assert((std::is_same<GetElem<pretty_num<1>::type, Tuple<std::true_type, std::false_type>>::type, std::false_type>::value), "{true, false}[1] == false");

	// isTwo
	static_assert((std::is_same<isTwo<pretty_num<2>::type>::type, std::true_type>::value), "2 is Two");
	// isEven
	static_assert((std::is_same<isEven<pretty_num<2>::type>::type, std::true_type>::value), "2 is Even");
	static_assert((std::is_same<isEven<pretty_num<1>::type>::type, std::false_type>::value), "1 is not Even");
	static_assert((std::is_same<isEven<pretty_num<3>::type>::type, std::false_type>::value), "3 is not Even");
	static_assert((std::is_same<isEven<pretty_num<10>::type>::type, std::true_type>::value), "10 is Even");

	// Collatz Next Function
	static_assert((std::is_same<next<pretty_num<0>::type>::type, Undefined>::value), "next(0) is Undefined");
	static_assert((std::is_same<next<pretty_num<2>::type>::type, pretty_num<1>::type>::value), "next(2) is 1");
	static_assert((std::is_same<next<pretty_num<4>::type>::type, pretty_num<2>::type>::value), "next(4) is 2");
	static_assert((std::is_same<next<pretty_num<1>::type>::type, pretty_num<4>::type>::value), "next(1) is 4");
	static_assert((std::is_same<next<pretty_num<3>::type>::type, pretty_num<10>::type>::value), "next(3) is 10");

	// Collatz Count Function
	static_assert((std::is_same<num_steps<pretty_num<1>::type>::type, pretty_num<0>::type>::value), "num_steps(1) is 0");
	static_assert((std::is_same<num_steps<pretty_num<0>::type>::type, Undefined>::value), "num_steps(0) is Undefined");
	static_assert((std::is_same<num_steps<pretty_num<2>::type>::type, pretty_num<1>::type>::value), "num_steps(2) is 1");
	static_assert((std::is_same<num_steps<pretty_num<3>::type>::type, pretty_num<7>::type>::value), "num_steps(3) is 7");
	
	// Accum
	assert(accum<pretty_num<3>::type>().inside == 3);
	// std::cout << accum<pretty_num<42>::type>().inside << std::endl;
	// std::cout << "num_steps(3) is " << accum<num_steps<pretty_num<3>::type>::type>().inside << std::endl;
	// In order to print at compile time we need a feature from gcc and this was done with clang. By the axiom of laziness we won't print at compile time

	// Runtime tests
	assert(sane_next(3) == 10);
	assert(sane_next(2) == 1);
	assert(sane_num_steps(3) == 7);
	assert(sane_recur_num_steps(3) == 7);
	
	std::cout << "While   : " << time_function(&sane_num_steps, test_value) << std::endl;
	std::cout << "Recurse : " << time_function(&sane_recur_num_steps, test_value) << std::endl;
	auto start = std::chrono::steady_clock::now();
	int ans;
	for (int i = 0; i < num_trials1; ++i)
	{
		ans = accum<num_steps<pretty_num<test_value>::type>::type>().inside;
	}
    std::cout << "Compile : " << ans << " : ";
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << elapsed_seconds.count() << std::endl;
	
	// Possible bug in clang at test_value 73?
	return 0;
}