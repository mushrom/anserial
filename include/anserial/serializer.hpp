#pragma once
#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <list>

namespace anserial {

class serializer {
	public:
		std::vector<uint32_t> output;
		std::map<uint32_t, std::string> symtab;

		// last assigned entity ID
		uint32_t ent_counter = 0;

		// primitives for adding to the tree
		uint32_t add_ent(uint32_t type, uint32_t parent, uint32_t data);
		uint32_t add_container(uint32_t parent);
		uint32_t add_symbol(uint32_t parent, const std::string& symbol);
		uint32_t add_symbol(uint32_t parent, uint32_t symbol);
		uint32_t add_integer(uint32_t parent, uint32_t data);
		uint32_t add_string(uint32_t parent, const std::string& str);
		uint32_t add_map(uint32_t parent);

		// convenience functions
		uint32_t add_version(uint32_t parent);
		uint32_t add_symtab(uint32_t parent);
		uint32_t add_data(uint32_t parent);

		// initializes an empty serializer to the default object layout,
		// with a top-level map and ::-prefixed metadata.
		// (eg. ::version, ::data, etc)
		uint32_t default_layout();

		// TODO: add a serialize() call to return data incrementally over multiple calls
		// TODO: output callback function, so we can write data as it's being serialized
		//       without buffering the full output
		std::vector<uint32_t> serialize() { return output; };

		// allows for neat ergonomic syntax, but a bit slower
		class ent_int {
			public:
				ent_int(uint32_t i);
				ent_int(std::string str);
				ent_int(const char* str);
				ent_int(std::list<ent_int> ents);
				ent_int(std::initializer_list<ent_int> ents);

				uint32_t id;
				uint32_t d_type;

				struct {
					uint32_t i;
					std::string s_str;
					std::initializer_list<ent_int> ents;
				} datas;
		};

		uint32_t add_entities(uint32_t parent, ent_int);
		uint32_t add_map_entry(uint32_t parent,
		                       const std::string& symbol,
		                       ent_int things);
};


// namespace anserial
}
