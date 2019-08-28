#pragma once

#include <anserial/base_ent.hpp>
#include <anserial/s_node.hpp>
#include <stdint.h>
#include <vector>

namespace anserial {

class deserializer {
	public:
		deserializer() {};
		deserializer(uint32_t *datas, size_t entities) {
			deserialize(datas, entities);
		}

		deserializer(std::vector<uint32_t> datas) {
			deserialize(datas);
		}

		// used for decoding
		std::vector<s_node*> nodes;

		// last assigned entity ID
		uint32_t ent_counter = 0;

		// returns just what is already parsed
		s_node *deserialize();

		// add entities to the deserialized tree
		s_node *deserialize(uint32_t *datas, size_t entities);
		s_node *deserialize(std::vector<uint32_t> datas);
};

// namespace anserial
}
