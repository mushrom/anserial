#pragma once

#include <anserial/version.hpp>
#include <stdint.h>
#include <list>
#include <vector>
#include <map>
#include <stdexcept>

namespace anserial {

typedef struct { uint32_t datas[2]; } serialized;

// type information
// note that this only uses 2 bits of information - used
// for tagging the serialized data
enum {
	ENT_TYPE_CONTAINER,
	ENT_TYPE_SYMBOL,
	ENT_TYPE_INTEGER,
	ENT_TYPE_STRING,
	ENT_TYPE_MAP,

	// TODO: dno't know if we'll keep these
	ENT_TYPE_SET,
	ENT_TYPE_NULL,
};

class s_ent {
	public:
		// entity ID number
		uint32_t id;
		// parent container ID
		uint32_t parent;
		// data buffer, always 4 bytes
		uint32_t data;
		// data type
		uint32_t d_type;
};

// TODO: move this somewhere better
uint32_t hash_string(const std::string& str);

// TODO: rename to different prefix so it's clear this is part of a tree class
// TODO: make an exception-free version of this
class s_node {
	public:
		virtual ~s_node() {}
		virtual void link_ent(s_node* ent) {
			// silently drop links to this node
			// TODO: maybe provide an error message, or have linking in the
			//       base class
		}

		// on a base node, there are no valid ways to access data. subclasses
		// of this will override these functions to provide ways to access data,
		// so we get runtime type checking without too much overhead.
		virtual s_node* get(uint32_t index){
			throw std::logic_error("anserial: no get(int) method for type " + type());
		};

		virtual s_node* get(std::string& symbol){
			throw std::logic_error("anserial: no get(symbol) method for type " + type());
		};

		virtual std::string& string(){
			throw std::logic_error("anserial: no string() method for type " + type());
		};

		virtual uint32_t uint(){
			throw std::logic_error("anserial: no uint() method for type " + type());
		};

		// returns a vector reference with the list of contained entities
		virtual std::vector<s_node*>& entities() {
			// return an empty vector by default, so we don't
			// need to do casts to get contained entities
			static std::vector<s_node*> void_vec = {};
			return void_vec;
		}

		// intended for use in maps, returns a list of keys to access entities
		virtual std::vector<s_node*>& keys() {
			static std::vector<s_node*> void_vec = {};
			return void_vec;
		}

		const std::string& type(void) {
			static const std::string types[] = {
				"container", "symbol", "integer", "string",
				"map", "set", "null",
			};

			return types[self.d_type];
		}

		// raw deserialized entity
		s_ent self;
};

class s_container : public s_node {
	public:
		virtual ~s_container() {
			for (auto& x : ents) {
				// self-recursive parents are allowed so we need to check for that here
				if (x != this) {
					delete x;
				}
			}
		}

		virtual s_node* get(uint32_t index) {
			if (index > ents.size()) {
				throw "out of range or something";
			}

			return ents[index];
		}

		virtual void link_ent(s_node* ent) {
			ents.push_back(ent);
		}

		virtual std::vector<s_node*>& entities() {
			return ents;
		}

		std::vector<s_node*> ents;
};

class s_map : public s_node {
	public:
		virtual ~s_map() {
			for (auto& x : entries) {
				// self-recursive parents are allowed so we need to check for that here
				if (x.second != this) {
					delete x.second;
				}
			}
		}

		virtual s_node* get(std::string& symbol){
			return entries[hash_string(symbol)];
		}

		virtual s_node* get(uint32_t symbol) {
			return entries[symbol];
		}

		virtual void link_ent(s_node* ent) {
			// XXX: for now, don't link to self, not sure what to do
			//      when a map is the top-level entity since the first added
			//      entry will be itself.
			// TODO: fix this
			if (ent == this) {
				return;
			}

			if (!have_sym) {
				// even links are keys for map entries
				last_sym = ent->self.data;
				have_sym = true;

				// TODO: it might be a good idea to set this to a null entity
				//       of some type
				// TODO: check that we actually have a symbol
				entries[last_sym] = nullptr;
				ent_keys.push_back(ent);
			}

			else {
				entries[last_sym] = ent;
				ents.push_back(ent);
				have_sym = false;
			}
		}

		// returns a vector reference with the list of contained entities
		virtual std::vector<s_node*>& entities() {
			return ents;
		}

		// intended for use in maps, returns a list of keys to access entities
		virtual std::vector<s_node*>& keys() {
			return ent_keys;
		}

		// state for link_ent()
		uint32_t last_sym = 0;
		// link_ent expects the first call to be a symbol entry,
		// and the second to be the map entry.
		// this keeps track of the current state.
		bool have_sym = false;

		// keep seperate vectors for keys/entries, for efficiency
		std::vector<s_node*> ent_keys;
		std::vector<s_node*> ents;

		// main map
		std::map<uint32_t, s_node*> entries;
};

class s_string : public s_node {
	public:
		virtual ~s_string() {}
		virtual void link_ent(s_node* ent) {
			if (ent && ent->self.d_type == ENT_TYPE_INTEGER) {
				str += ent->self.data;
			}
		}

		virtual std::string& string() {
			return str;
		}

		std::string str;
};

class s_uint : public s_node {
	public:
		virtual ~s_uint() {}

		virtual uint32_t uint(){
			return self.data;
		}
};

class s_symbol : public s_node {
	public:
		virtual ~s_symbol() { };

		virtual uint32_t uint(){
			return self.data;
		}
};

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

class deserializer {
	public:
		// used for decoding
		std::vector<s_node*> nodes;

		// last assigned entity ID
		uint32_t ent_counter = 0;

		s_node *deserialize(uint32_t *datas, size_t entities);
		s_node *deserialize(std::vector<uint32_t> datas);
};

// XXX
void dump_nodes(s_node *node, unsigned indent);

// namespace anserial
}
