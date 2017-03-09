
#include "du1example.hpp"
#include "du1examplepolicy.hpp"
#include "du1serialize.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char * * argv)
{
	testA xA{ 100, "Hello" };
	xA.fA3().push_back({ "One", testB::E_X });
	xA.fA3().push_back({ "Two", testB::E_Y });
	xA.fA3().push_back({ "Three", testB::E_Z });

	std::string fname = "xA.json";
	{
		std::ofstream ofs{ fname };
		serialize::dump< structpolicyA>(ofs, (const testA &)xA);
	}

	{
		std::ifstream ifs{ fname };
		auto xAcopy = serialize::load< structpolicyA>(ifs);

		serialize::dump< structpolicyA>(std::cerr, xAcopy);
	}

	return 0;
}

