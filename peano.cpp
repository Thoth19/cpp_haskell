#include <iostream>
#include <cassert>

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

	return 0;
}