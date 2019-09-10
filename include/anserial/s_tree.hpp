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

		// metadata accessors
		s_node *data();

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

		struct {
			// top-level node
			s_node *top;
			// map node with symbol info
			s_node *symtab;
			// generator version
			s_node *version;
			// main content
			s_node *data;
		} cached = {nullptr, nullptr, nullptr, nullptr};
};

// namespace anserial
}
