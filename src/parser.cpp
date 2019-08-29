#include <anserial/parser.hpp>
#include <string.h>
#include <iostream>

using namespace anserial;

s_tree sexp_parser::parse(void) {
	token foo = parse_top();
	return s_tree(foo.node);
}

token sexp_parser::parse_top(void) {
	// TODO: accept any token type at the top level
	token top = parse_container();
	expect(token::types::EndOfFile);

	return top;
}

token sexp_parser::parse_container(void) {
	s_container *container = new s_container;

	token ret;
	ret.type = token::types::List;
	ret.node = container;
	ret.node->self.d_type = ENT_TYPE_CONTAINER;
	ret.node->self.id = ent_counter++;

	expect(token::types::OpenParen);
	consume();

	while (accept(token::types::OpenParen)
	    || accept(token::types::Symbol)
	    || accept(token::types::Int)
	    || accept(token::types::String))
	{
		token foo;

		if (accept(token::types::OpenParen)) {
			foo = parse_container();

		} else {
			foo = get_token();
		}

		container->link_ent(foo.node);
	}

	expect(token::types::CloseParen);
	consume();

	return ret;
}

bool sexp_parser::accept(token::types type) {
	return peek_token().type == type;
}

bool sexp_parser::expect(token::types type) {
	if (peek_token().type != type) {
		// TODO: use a better error type here
		throw std::logic_error("invalid syntax: line " + std::to_string(line));
	}

	return true;
}

void sexp_parser::consume(void) {
	// get a token and throw it away
	get_token();
}

token sexp_parser::get_token(void) {
	if (tokens.empty()) {
		return read_token();
	}

	else {
		token ret = tokens.front();
		tokens.pop_front();
		return ret;
	}
}

void sexp_parser::push_token(token t) {
	tokens.push_front(t);
}

token sexp_parser::peek_token(void) {
	// instead of peeking the token list directly, we get_token()
	// here so that we never run into a situation where the token
	// list is empty (since we'll grab a new one from input if it is)
	token ret = get_token();
	push_token(ret);
	return ret;
}

void sexp_parser::read_whitespace(void) {
	char c = 0;

	// consume whitespace
	while ((c == '\0' || c == '\r' || c == '\n' || c == '\t'
	     || c == ' ' || c == '\v' || c == '\a' || c == ';')
	     && !feof(file))
	{
		if (c == '\n') {
			line++;
		}

		if (c == ';') {
			read_comment();
		}

		c = fgetc(file);
	}

	if (!feof(file)) {
		ungetc(c, file);
	}
}

void sexp_parser::read_comment(void) {
	char c = fgetc(file);

	while (c != '\n') {
		c = fgetc(file);
	}

	if (!feof(file)) {
		ungetc(c, file);
	}
}

token sexp_parser::read_token(void) {
	token ret;

	read_whitespace();
	char c = fgetc(file);

	if (feof(file)) {
		ret.type = token::types::EndOfFile;
	}

	else if (c == '(') {
		ret.type = token::types::OpenParen;
	}

	else if (c == ')') {
		ret.type = token::types::CloseParen;
	}

	else if (c == '"') {
		ret.type = token::types::String;
		s_string *temp = new s_string;

		char c = fgetc(file);
		while (c != '"') {
			// TODO: escapes
			temp->str += c;
			c = fgetc(file);
		}

		ret.node = temp;
		ret.node->self.d_type = ENT_TYPE_STRING;
		ret.node->self.id = ent_counter++;
	}

	// TODO: if first char is '-', check next char to determine
	//       whether it's an int or a symbol
	else if (c >= '0' && c <= '9') {
		ret.type = token::types::Int;
		s_uint *temp = new s_uint;

		std::string foo;
		while (c >= '0' && c <= '9') {
			foo += c;
			c = fgetc(file);
		}

		if (!feof(file)) {
			ungetc(c, file);
		}

		temp->self.data = stoi(foo);
		ret.node = temp;
		// TODO: get rid of these d_type initializations
		ret.node->self.d_type = ENT_TYPE_INTEGER;
		ret.node->self.id = ent_counter++;
	}

	else if ((c >= 'a' && c <= 'z')
		  || (c >= 'A' && c <= 'Z')
		  || (strchr("!@#$%^&*-=|~+", c)))
	{
		ret.type = token::types::Symbol;
		s_symbol *temp = new s_symbol;

		std::string foo;
		while ((c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| (strchr("!@#$%^&*-=|~+:<>?.,/", c))
			|| (c >= '0' && c <= '9'))
		{
			foo += c;
			c = fgetc(file);
		}

		if (!feof(file)) {
			ungetc(c, file);
		}

		// TODO: we should store the string also, so we can
		//       properly generate a symtab when serializing
		//       the tree
		temp->self.data = hash_string(foo);
		ret.node = temp;
		ret.node->self.d_type = ENT_TYPE_SYMBOL;
		ret.node->self.id = ent_counter++;
	}

	return ret;
}
