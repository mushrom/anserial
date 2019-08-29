#pragma once

#include <anserial/s_node.hpp>
#include <anserial/deserializer.hpp>

namespace anserial {

class s_tree {
	public:
		// TODO: smarter pointers
		s_tree();
		s_tree(s_node *node);
		s_tree(deserializer* nder);

		s_node *lookup(std::string& symbol);
		s_node *lookup(uint32_t hash);

		// update cached meta-objects, in case the deserializer
		// parsed new entities
		void refresh(void);
		void dump_nodes(void);
		void dump_nodes(s_node *node, unsigned indent=0);

	private:
		// TODO: should use a smart pointer here
		deserializer *der = nullptr;

		// top-level node
		s_node *top = nullptr;
		// map node with symbol info
		s_node *symtab = nullptr;
		// generator version
		s_node *version = nullptr;
		// main content
		s_node *data = nullptr;
};

// namespace anserial
}
