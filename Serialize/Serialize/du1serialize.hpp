#ifndef du1serialize_hpp_
#define du1serialize_hpp_

#include <istream>
#include <ostream>
#include <vector>
#include <utility>
#include <string>

namespace serialize {

	template< typename TObj>
	struct dump_pair {
		std::ostream & out;
		const TObj & obj;
	};

	template< typename TObj>
	struct load_pair {
		std::istream & in;
		TObj & obj;
	};

	char ignore_whitespace(std::istream & in);
	char ignore_identifier(std::istream & in);
	std::int_least32_t parse_int_attribute(std::istream & in);
	std::string parse_string_attribute(std::istream & in);

	// you have to implement the following functions:

	template< typename P, typename T>
	inline void dump(std::ostream & ofs, T && t)
	{
		dump_pair<T> & pair = dump_pair<T>{ofs,t};
		ofs << "{" << std::endl;
		P::make(pair);
		ofs << "}";
	}

	template< typename P>
	inline auto load(std::istream & ifs)
	{
		auto t = P::create();
		auto pair = load_pair<decltype(t)>{ ifs, t };

		// object should start with left curly brace
		if (!ifs.eof()) {
			char ch;
			ifs >> ch;
			if (ch != '{') {
				throw std::exception("Incorrect syntax");
			}
		}
		else {
			throw std::exception("Incorrect syntax");
		}

		P::make(pair);
		/*
		// object should end on right curly brace
		if (!ifs.eof()) {
			char ch = ignore_whitespace(ifs);
			if (ch != '}') {
				throw std::exception("Incorrect syntax");
			}
		}
		else {
			throw std::exception("Incorrect syntax");
		}*/
		return t;
	}
	
	// to support the original du1examplepolicy.hpp, you need to implement these types and functions:

	template< typename E>
	using enumeration_list = std::vector< std::pair< E, std::string>>;	
	
	template< typename E, typename F>
	inline void int_attribute(dump_pair<E> & e, std::string name, F && f)
	{
		// dump name
		e.out << "\"" << name << "\": ";
		// dump value
		e.out << f(&e.obj) << "," << std::endl;
	}

	template< typename E, typename F>
	inline void int_attribute(load_pair<E> & e, std::string name, F && f)
	{
		auto value = parse_int_attribute(e.in);
		f(&e.obj) = value;
	}

	template< typename E, typename F, typename S>
	inline void int_attribute(dump_pair<E> & e, std::string name, F && f, S && s)
	{
		e.out << "\"" << name << "\": ";
		e.out << f(&e.obj) << "," << std::endl;
	}

	template< typename E, typename F, typename S>
	inline void int_attribute(load_pair<E> & e, std::string name, F && f, S && s)
	{
		auto value = parse_int_attribute(e.in);
		s(&e.obj, value);
	}

	template< typename P, typename E, typename F>
	inline void enum_attribute(dump_pair<E> & e, std::string name, F && f)
	{
		// ...
	}

	template< typename P, typename E, typename F>
	inline void enum_attribute(load_pair<E> & e, std::string name, F && f)
	{
		// ...
	}

	template< typename E, typename F>
	inline void string_attribute(dump_pair<E> & e, std::string name, F && f)
	{
		e.out << "\"" << name << "\": \"";		
		e.out << f(&e.obj) << "\"," << std::endl;
	}

	template< typename E, typename F>
	inline void string_attribute(load_pair<E> & e, std::string name, F && f)
	{
		auto value = parse_string_attribute(e.in);
		f(&e.obj) = value;
	}

	template< typename P, typename E, typename F>
	inline void struct_sequence_attribute(dump_pair<E> & e, std::string name, F && f)
	{
		e.out << "\"" << name << "\": [" << std::endl;
		auto seq = f(&e.obj);
		for (auto i = seq.begin(); i != seq.end(); ++i) {
			dump<P>(e.out, *i);
			e.out << "," << std::endl;
		}
		e.out << "]," << std::endl;
	}

	template< typename P, typename E, typename F>
	inline void struct_sequence_attribute(load_pair<E> & e, std::string name, F && f)
	{
		// ...
	}
}

#endif
