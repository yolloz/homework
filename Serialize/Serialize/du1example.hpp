#ifndef du1example_hpp_
#define du1example_hpp_

#include <string>
#include <list>

class testB {
public:
	std::string attrB1;
	enum E { E_X, E_Y, E_Z } attrB2;
};

class testA {
public:
	testA(int a1, const std::string & a2)
		: attrA1(a1), attrA2(a2)
	{}
	int get_A1() const
	{
		return attrA1;
	}
	void set_A1( int x)
	{
		attrA1 = x;
	}
	std::string & fA2()
	{
		return attrA2;
	}
	const std::string & fA2() const
	{
		return attrA2;
	}
	std::list< testB> & fA3()
	{
		return attrA3;
	}
	const std::list< testB> & fA3() const
	{
		return attrA3;
	}
private:
	int attrA1;
	std::string attrA2;
	std::list< testB> attrA3;
};

#endif
