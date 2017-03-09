#include "du1serialize.hpp"

namespace serialize {
	char ignore_whitespace(std::istream & in) {
		char ch;
		while (!in.eof()) {
			in >> ch;
			if (!isspace(ch)) {
				break;
			}
		}
		if (in.eof()) {
			throw std::exception("Unexpected end of stream");
		}
		else {
			return ch;
		}
	}

	char ignore_identifier(std::istream & in) {
		char ch;
		while (!in.eof()) {
			in >> ch;
			if (ch == '"') {
				break;
			}
		}
		if (in.eof()) {
			// throw exception
			throw std::exception("Unexpected end of stream");
		}
		else {
			return ch;
		}
	}

	std::int_least32_t parse_int_attribute(std::istream & in) {
		// find start of attribute name
		char ch;
		ch = ignore_whitespace(in);
		if (ch == '"') {
			// idf name not important
			ch = ignore_identifier(in);
			if (ch == '"') {
				ch = ignore_whitespace(in);
				if (ch == ':') {
					ch = ignore_whitespace(in);
					bool negative = false;
					if (ch == '-') {
						negative = true;
						if (!in.eof()) {
							in >> ch;
						}
					}
					if (isdigit(ch)) {
						int_least32_t value = 0;
						value = (int)ch - 48;
						while (!in.eof()) {
							in >> ch;
							if (isdigit(ch)) {
								value *= 10;
								value += (int)ch - 48;
							}
							else {
								break;
							}
						}
						if (ch == ',') {
							if (negative) {
								value = -value;
							}
							return value;
						}
					}
				}
			}
		}
		// invalid input, throw exception
		throw std::exception("Incorrect syntax detected during parsing of integer attribute");
	}

	std::string parse_string_attribute(std::istream & in) {
		// find start of attribute name
		char ch;
		ch = ignore_whitespace(in);
		if (ch == '"') {
			// idf name not important
			ch = ignore_identifier(in);
			if (ch == '"') {
				ch = ignore_whitespace(in);
				if (ch == ':') {
					ch = ignore_whitespace(in);
					if (ch == '"') {
						std::string value = "";
						while (!in.eof()) {
							in >> ch;
							if (ch != '"') {
								value += ch;
							}
							else {
								break;
							}
						}
						if (ch == '"') {
							ch = ignore_whitespace(in);
							if (ch == ',') {
								return value;
							}
						}
					}
				}
			}
		}
		// invalid input, throw exception
		throw std::exception("Incorrect syntax detected during parsing of string attribute");
	}
}

