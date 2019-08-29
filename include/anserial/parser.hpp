#pragma once

#include <anserial/s_node.hpp>
#include <anserial/s_tree.hpp>
#include <anserial/deserializer.hpp>
#include <list>
#include <stdio.h>

namespace anserial {

class token {
	public:
		enum types {
			// character entities 
			OpenParen,
			CloseParen,

			// Data tokens, which have corresponding 
			List,
			Symbol,
			Int,
			String,

			// file stream token
			EndOfFile,
		} type;

		// valid pointer for data tokens, nullptr for other tokens
		s_node *node;
};

class sexp_parser {
	public:
		sexp_parser(FILE *fp) {
			file = fp;
		}

		s_tree parse(void);

	private:
		bool accept(token::types type);
		bool expect(token::types type);
		void consume(void);

		token get_token(void);
		token read_token(void);
		token peek_token(void);
		void push_token(token t);
		void read_whitespace(void);
		void read_comment(void);

		token parse_top(void);
		token parse_container(void);

		// XXX: for now, just use C stdio stuff, later we might use
		//      string streams or whatever, we only need lookahead of
		//      1 anyway
		FILE *file;
		std::list<token> tokens;
		unsigned ent_counter = 0;
		unsigned line = 0;
};

// namespace anserial
}
