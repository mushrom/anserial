#include <anserial/anserial.hpp>
#include <list>
#include <vector>
#include <map>
#include <initializer_list>
#include <stdint.h>
#include <stdexcept>

// for htonl/ntohl
#include <arpa/inet.h>

namespace anserial {

serialized serialize_ent(const s_ent& ent) {
	serialized ret;

	ret.datas[0] = htonl((ent.d_type << 30) | ent.parent);
	ret.datas[1] = htonl(ent.data);

	return ret;
}

s_ent deserialize_ent(const serialized& ser) {
	s_ent ret;

	ret.d_type = ntohl(ser.datas[0]) >> 30;
	ret.parent = ntohl(ser.datas[0]) & ~(3 << 30);
	ret.data   = ntohl(ser.datas[1]);

	return ret;
}

s_node *deserializer::deserialize(uint32_t *datas, size_t entities) {
	for (unsigned i = 0; i < entities; i++) {
		serialized buf;
		buf.datas[0] = datas[2*i];
		buf.datas[1] = datas[2*i + 1];

		s_node *temp = new s_node;
		s_ent entity = deserialize_ent(buf);

		if (entity.parent > ent_counter) {
			throw std::out_of_range("deserializer::deserialize(): parent ID is invalid");
		}

		temp->self = entity;
		temp->self.id = ent_counter++;

		nodes.push_back(temp);

		// link node up to parent node
		nodes[temp->self.parent]->entities.push_back(temp);
	}

	return nodes[0];
}

s_node *deserializer::deserialize(std::vector<uint32_t> datas) {
	return deserialize(datas.data(), datas.size() / 2);
}

serializer::ent_int::ent_int(uint32_t i) {
	d_type = ENT_TYPE_INTEGER;
	datas.i = i;
}

serializer::ent_int::ent_int(std::string str) {
	d_type = ENT_TYPE_SYMBOL;
	datas.s_str = str;
}

serializer::ent_int::ent_int(const char *str) {
	d_type = ENT_TYPE_SYMBOL;
	datas.s_str = std::string(str);
}

serializer::ent_int::ent_int(std::initializer_list<serializer::ent_int> ents) {
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

uint32_t hash_string(const std::string& str) {
	unsigned hash = 19937;

	for (char c : str) {
		hash = (hash << 7) + hash + c;
	}

	return hash;
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

uint32_t serializer::add_version(uint32_t parent) {
	return add_entities(parent,
		{"::version",
			{"major", version.major},
			{"minor", version.minor},
			{"patch", version.patch}});;
}

uint32_t serializer::add_symtab(uint32_t parent) {
	uint32_t cont = add_container(parent);

	add_symbol(cont, "::symtab");

	for (const auto& x : symtab) {
		uint32_t entry = add_container(cont);
		add_symbol(entry, x.first);
		add_string(entry, x.second);
	}

	return cont;
}

// namespace simple_serializer
}

using namespace anserial;

void dump_nodes(s_node *node, unsigned indent) {
	const char *types[] = {"container", "symbol", "integer", "string"};
	printf("%*s(%s", 4*indent, " ", types[node->self.d_type]);

	switch (node->self.d_type) {
		case ENT_TYPE_CONTAINER:
			break;

		case ENT_TYPE_STRING:
			printf(" ; \"");
			for (const auto& x : node->entities) {
				putchar(x->self.data);
			}
			putchar('"');
			break;

		case ENT_TYPE_SYMBOL:
			printf(" #x%x", node->self.data);
			break;

		case ENT_TYPE_INTEGER:
			printf(" %u", node->self.data);
			break;
	}

	for (auto& x : node->entities) {
		if (x->self.id != node->self.id) {
			putchar('\n');
			dump_nodes(x, indent + 1);
		}
	}

	putchar(')');

	if (indent == 0) {
		putchar('\n');
	}
}

int main(int argc, char *argv[]) {
	serializer foo;

	// add empty container at the top level
	uint32_t top = foo.add_container(0);
	foo.add_version(top);

	uint32_t counting = foo.add_entities(top, {});
	for (unsigned i = 0; i < 10000; i++) {
		foo.add_entities(counting,
			{"results",
				{"i-19937", i*19937},
				{"i-2048",  i*2048}});

		/*
		// does the same as above, but with primitive add functions.
		// this is a bit faster, since we don't instantiate a tree
		// before adding it.
		uint32_t k = foo.add_container(counting);
		foo.add_symbol(k, "results");

		uint32_t cont = foo.add_container(k);
		foo.add_symbol(cont, "i*19937");
		foo.add_integer(cont, i*19937);

		cont = foo.add_container(k);
		foo.add_symbol(cont, "i*2048");
		foo.add_integer(cont, i*2048);
		*/
	}

	foo.add_symtab(top);

	auto buf = foo.serialize();

	if (argc > 1) {
		deserializer der;
		auto foo = der.deserialize(buf);
		dump_nodes(foo, 0);
		delete foo;

	} else {
		fwrite(buf.data(), 4, buf.size(), stdout);
	}

	return 0;
}
