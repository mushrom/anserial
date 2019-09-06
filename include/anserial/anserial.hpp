#pragma once

#include <anserial/version.hpp>
#include <anserial/base_ent.hpp>

// TODO: this is only needed by the deserializer class, remove this
//       once that's split
#include <anserial/serializer.hpp>
#include <anserial/deserializer.hpp>
#include <anserial/s_node.hpp>
#include <anserial/s_tree.hpp>

namespace anserial {

// XXX
// TODO: s_tree class
void dump_nodes(s_node *node, unsigned indent);

bool destructure(s_node *node, ent_int ents);

// namespace anserial
}
