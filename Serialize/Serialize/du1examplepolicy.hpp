#ifndef du1examplepolicy_hpp_
#define du1examplepolicy_hpp_

#include "du1example.hpp"
#include "du1serialize.hpp"

struct enumpolicyE
{
	static auto create()
	{
		return testB::E{};
	}

	static serialize::enumeration_list< testB::E> enumeration()
	{
		return{
			{ testB::E_X, "X" },
			{ testB::E_Y, "Y" },
			{ testB::E_Z, "Z" }
		};
	}
};

struct structpolicyB
{
	static auto create() {
		return testB{};
	}

	template< typename X>
	static void make(X && x) {
		serialize::string_attribute(x, "B1", [](auto && m) -> auto && { return m->attrB1; });
		serialize::enum_attribute< enumpolicyE>(x, "B2", [](auto && m) -> auto && { return m->attrB2; });
	}
};

struct structpolicyA
{
	static auto create() {
		return testA{ 0, "" };
	}

	template< typename X>
	static void make(X && x) {
		serialize::int_attribute(x, "A1", [](auto && m) { return m->get_A1(); }, [](auto && m, auto && v) { m->set_A1(v); });
		serialize::string_attribute(x, "A2", [](auto && m) -> auto && { return m->fA2(); });
		serialize::struct_sequence_attribute< structpolicyB>(x, "A3", [](auto && m) -> auto && { return m->fA3(); });
	}
};

#endif
