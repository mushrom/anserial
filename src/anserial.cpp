#include <anserial/anserial.hpp>
#include <list>
#include <vector>
#include <map>
#include <initializer_list>
#include <stdint.h>
#include <stdexcept>
#include <iostream>

// for htonl/ntohl
#include <arpa/inet.h>

namespace anserial {

serialized serialize_ent(const s_ent& ent) {
	serialized ret;

	ret.datas[0] = htonl((ent.d_type << 29) | ent.parent);
	ret.datas[1] = htonl(ent.data);

	return ret;
}

s_ent deserialize_ent(const serialized& ser) {
	s_ent ret;

	ret.d_type = ntohl(ser.datas[0]) >> 29;
	ret.parent = ntohl(ser.datas[0]) & ~(7 << 29);
	ret.data   = ntohl(ser.datas[1]);

	return ret;
}

// TODO: move this somewhere better
uint32_t hash_string(const std::string& str) {
	unsigned hash = 19937;

	for (char c : str) {
		hash = (hash << 7) + hash + c;
	}

	return hash;
}

s_node *deserializer::deserialize() {
	return nodes[0];
}

s_node *deserializer::deserialize(uint32_t *datas, size_t entities) {
	for (unsigned i = 0; i < entities; i++) {
		serialized buf;
		buf.datas[0] = datas[2*i];
		buf.datas[1] = datas[2*i + 1];

		s_ent entity = deserialize_ent(buf);
		s_node *temp;

		switch (entity.d_type) {
			case ENT_TYPE_CONTAINER: temp = new s_container; break;
			case ENT_TYPE_MAP:       temp = new s_map; break;
			case ENT_TYPE_STRING:    temp = new s_string; break;
			case ENT_TYPE_SYMBOL:    temp = new s_symbol; break;
			case ENT_TYPE_INTEGER:   temp = new s_uint; break;

			default: temp = new s_node; break;
		}

		if (entity.parent > ent_counter) {
			throw std::out_of_range("deserializer::deserialize(): parent ID is invalid");
		}

		temp->self = entity;
		temp->self.id = ent_counter++;

		nodes.push_back(temp);

		// link node up to parent node
		//nodes[temp->self.parent]->entities.push_back(temp);
		nodes[temp->self.parent]->link_ent(temp);
	}

	return nodes[0];
}

s_node *deserializer::deserialize(std::vector<uint32_t> datas) {
	return deserialize(datas.data(), datas.size() / 2);
}

ent_int::ent_int(uint32_t i) {
	d_type = ENT_TYPE_INTEGER;
	datas.i = i;
}

ent_int::ent_int(std::string str) {
	d_type = ENT_TYPE_SYMBOL;
	datas.s_str = str;
}

ent_int::ent_int(uint32_t *an_uptr) {
	d_type = ENT_TYPE_UINT_PTR;
	datas.uptr = an_uptr;
}

ent_int::ent_int(std::string *an_sptr) {
	d_type = ENT_TYPE_STRING_PTR;
	datas.sptr = an_sptr;
}

ent_int::ent_int(s_node **an_nptr) {
	d_type = ENT_TYPE_NODE_PTR;
	datas.nptr = an_nptr;
}

ent_int::ent_int(const char *str) {
	d_type = ENT_TYPE_SYMBOL;
	datas.s_str = std::string(str);
}

ent_int::ent_int(std::initializer_list<ent_int> ents) {
	d_type = ENT_TYPE_CONTAINER;

	datas.ents = ents;
}

uint32_t serializer::add_entities(uint32_t parent, ent_int ent) {
	switch (ent.d_type) {
		case ENT_TYPE_CONTAINER:
			{
				uint32_t id = add_container(parent);
				for (auto& x : ent.datas.ents) {
					add_entities(id, x);
				}
				return id;
			}

		case ENT_TYPE_SYMBOL:
			return add_symbol(parent, ent.datas.s_str);

		case ENT_TYPE_INTEGER:
			return add_integer(parent, ent.datas.i);

		default:
			return 0;
	}

	return 0;
}

uint32_t serializer::add_ent(uint32_t type, uint32_t parent, uint32_t data) {
	if (parent > ent_counter) {
		throw std::out_of_range("serializer::add_ent(): parent ID is invalid");
	}

	uint32_t ret = ent_counter++;

	s_ent ent;
	ent.d_type = type;
	ent.id     = ret;
	ent.parent = parent;
	ent.data   = data;

	serialized buf = serialize_ent(ent);
	output.push_back(buf.datas[0]);
	output.push_back(buf.datas[1]);

	return ret;
}

uint32_t serializer::add_container(uint32_t parent) {
	return add_ent(ENT_TYPE_CONTAINER, parent, 0);
}

uint32_t serializer::add_symbol(uint32_t parent, const std::string& symbol) {
	uint32_t hash = hash_string(symbol);
	symtab[hash] = symbol;

	uint32_t ret = add_ent(ENT_TYPE_SYMBOL, parent, hash);

	return ret;
}

uint32_t serializer::add_symbol(uint32_t parent, uint32_t symbol) {
	// Raw symbol addition, used internally for generating symbol tables, but might
	// be useful elsewhere.
	return add_ent(ENT_TYPE_SYMBOL, parent, symbol);
}

uint32_t serializer::add_integer(uint32_t parent, uint32_t data) {
	return add_ent(ENT_TYPE_INTEGER, parent, data);
}

uint32_t serializer::add_string(uint32_t parent, const std::string& str) {
	uint32_t ret = add_ent(ENT_TYPE_STRING, parent, 0xbadf00d);

	uint32_t str_id = ret;
	for (char c : str) {
		// add characters as codepoints, this way unicode
		// can be fitted in easily enough
		add_integer(str_id, c);
	}

	return ret;
}

uint32_t serializer::add_map(uint32_t parent) {
	return add_ent(ENT_TYPE_MAP, parent, 0xcafebabe);
}

uint32_t serializer::add_map_entry(uint32_t parent,
                                   const std::string& symbol,
                                   ent_int things)
{
	add_symbol(parent, symbol);
	return add_entities(parent, things);
}

uint32_t serializer::add_version(uint32_t parent) {
	return add_map_entry(parent, "::version", {
		{"major", version.major},
		{"minor", version.minor},
		{"patch", version.patch}
	});
}

uint32_t serializer::add_data(uint32_t parent) {
	return add_map_entry(parent, "::data", {});
}

uint32_t serializer::add_symtab(uint32_t parent) {
	// assumes the parent is a map itself
	add_symbol(parent, "::symtab");
	uint32_t cont = add_map(parent);

	for (const auto& x : symtab) {
		//uint32_t entry = add_container(cont);
		add_symbol(cont, x.first);
		add_string(cont, x.second);
	}

	return cont;
}

uint32_t serializer::default_layout() {
	uint32_t top = add_map(0);
	add_version(top);
	return add_data(top);
}

// TODO: might be a good idea to split this up into a few
//       mutually-recursive functions
bool destructure(s_node *node, ent_int ent) {
	if (ent.d_type == ENT_TYPE_NODE_PTR) {
		if (ent.datas.nptr) {
			*ent.datas.nptr = node;
		}

		// TODO: should we really return true in all cases?
		return true;
	}

	else if (!node) {
		return false;
	}

	else if (node->self.d_type == ENT_TYPE_CONTAINER
		&& ent.d_type == ENT_TYPE_CONTAINER)
	{
		auto it = ent.datas.ents.begin();

		for (unsigned i = 0; i < ent.datas.ents.size(); i++, it++) {
			if (!destructure(node->get(i), *it)) {
				return false;
			}
		}

		return true;
	}

	else if (node->self.d_type == ENT_TYPE_MAP
	         && ent.d_type == ENT_TYPE_CONTAINER)
	{
		auto it = ent.datas.ents.begin();

		while (it != ent.datas.ents.end()) {
			ent_int key = *it++;
			ent_int pattern = *it++;

			if (key.d_type != ENT_TYPE_SYMBOL) {
				return false;
			}

			if (!destructure(node->get(key.datas.s_str), pattern)) {
				return false;
			}
		}

		return true;
	}

	else if (node->self.d_type == ent.d_type) {
		switch (ent.d_type) {
			case ENT_TYPE_INTEGER:
				return node->uint() == ent.datas.i;

			case ENT_TYPE_SYMBOL:
				return node->uint() == hash_string(ent.datas.s_str);

			case ENT_TYPE_STRING:
				return node->string() == ent.datas.s_str;

			default:
				return false;
		}
	}

	else if (node->self.d_type == ENT_TYPE_INTEGER
	         && ent.d_type == ENT_TYPE_UINT_PTR)
	{
		// FIXME: type conversions result in wrong but consistent results...
		// TODO:  find out why, add test case once test framework is up
		//*ent.datas.uptr = *node;
		*ent.datas.uptr = node->uint();
		return true;
	}

	else if (node->self.d_type == ENT_TYPE_STRING
	         && ent.d_type == ENT_TYPE_STRING_PTR)
	{
		//*ent.datas.sptr = (std::string)*node;
		*ent.datas.sptr = node->string();
		return true;
	}

	return false;
}

/*
// TODO: make this part of s_node/s_tree classes
void dump_nodes(s_node *node, unsigned indent) {
}
*/

// namespace simple_serializer
}
