#pragma once

#include <anserial/base_ent.hpp>
#include <anserial/s_node.hpp>
#include <stdint.h>
#include <vector>

namespace anserial {

class deserializer {
	public:
		// used for decoding
		std::vector<s_node*> nodes;

		// last assigned entity ID
		uint32_t ent_counter = 0;

		s_node *deserialize(uint32_t *datas, size_t entities);
		s_node *deserialize(std::vector<uint32_t> datas);
};

// namespace anserial
}
