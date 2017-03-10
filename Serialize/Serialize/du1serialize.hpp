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
		
		// object should end on right curly brace
		if (!ifs.eof()) {
			char ch = ignore_whitespace(ifs);
			if (ch != '}') {
				throw std::exception("Incorrect syntax");
			}
		}
		else {
			throw std::exception("Incorrect syntax");
		}
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
		e.out << "\"" << name << "\": \"";
		auto enm = P::enumeration();
		auto value = f(&e.obj);
		bool found = false;
		for (auto i = enm.begin(); i != enm.end(); ++i) {
			if (i->first == value) {
				e.out << i->second << "\"," << std::endl;
				found = true;
				break;
			}
		}
		if (!found) {
			throw std::exception("Enumeration value was not contained in given policy class");
		}
	}

	template< typename P, typename E, typename F>
	inline void enum_attribute(load_pair<E> & e, std::string name, F && f)
	{
		// find start of attribute name
		char ch;
		ch = ignore_whitespace(e.in);
		if (ch == '"') {
			// idf name not important
			ch = ignore_identifier(e.in);
			if (ch == '"') {
				ch = ignore_whitespace(e.in);
				if (ch == ':') {
					ch = ignore_whitespace(e.in);
					if (ch == '"') {
						std::string value = "";
						while (!e.in.eof()) {
							e.in >> ch;
							if (ch != '"') {
								value += ch;
							}
							else {
								break;
							}
						}
						if (ch == '"') {
							ch = ignore_whitespace(e.in);
							if (ch == ',') {
								auto rtc = P::create();
								auto enm = P::enumeration();
								bool found = false;
								for (auto i = enm.begin(); i != enm.end(); ++i) {
									if (i->second == value) {
										rtc = i->first;
										found = true;
										break;
									}
								}
								if (found) {
									f(&e.obj) = rtc;
									return;
								}
							}
						}
					}
				}
			}
		}
		// invalid input, throw exception
		throw std::exception("Incorrect syntax detected during parsing of enum attribute");
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
		// find start of attribute name
		char ch;
		ch = ignore_whitespace(e.in);
		if (ch == '"') {
			// idf name not important
			ch = ignore_identifier(e.in);
			if (ch == '"') {
				ch = ignore_whitespace(e.in);
				if (ch == ':') {
					ch = ignore_whitespace(e.in);
					if (ch == '[') {
						auto seq = f(&e.obj);
						while (!e.in.eof()) {
							auto obj = load<P>(e.in);
							seq.push_back(obj);
							ch = ignore_whitespace(e.in);
							if (ch != ',') {
								break;
							}
							while (isspace(e.in.peek())) {
								e.in.get();
							}
							if (e.in.peek() == ']') {
								e.in.get();
								ch = ignore_whitespace(e.in);
								if (ch == ',') {
									f(&e.obj) = seq;
									return;
								}
							}
						}
					}
				}
			}
		}
		// invalid input, throw exception
		throw std::exception("Incorrect syntax detected during parsing of struct sequence attribute");
	}
}

#endif
