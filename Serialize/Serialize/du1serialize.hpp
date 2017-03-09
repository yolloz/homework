#ifndef du1serialize_hpp_
#define du1serialize_hpp_

#include <istream>
#include <ostream>
#include <vector>
#include <utility>
#include <string>

namespace serialize {

	// you have to implement the following functions:

	template< typename P, typename T>
	inline void dump(std::ostream & ofs, T && t)
	{
		// ...
	}

	template< typename P>
	inline auto load(std::istream & ifs)
	{
		auto t = P::create();

		// ...

		return t;
	}
	
	// to support the original du1examplepolicy.hpp, you need to implement these types and functions:

	template< typename E>
	using enumeration_list = std::vector< std::pair< E, std::string>>;	
	
	template< typename E, typename F>
	inline void int_attribute(E && e, std::string name, F && f)
	{
		// ...
	}

	template< typename E, typename F, typename S>
	inline void int_attribute(E && e, std::string name, F && f, S && s)
	{
		// ...
	}

	template< typename P, typename E, typename F>
	inline void enum_attribute(E && e, std::string name, F && f)
	{
		// ...
	}

	template< typename E, typename F>
	inline void string_attribute(E && e, std::string name, F && f)
	{
		// ...
	}

	template< typename P, typename E, typename F>
	inline void struct_sequence_attribute(E && e, std::string name, F && f)
	{
		// ...
	}
}

#endif
