// definition of low-level binary entities and constants
#pragma once
#include <stdint.h>
#include <string>

namespace anserial {

// TODO: move this somewhere better
//       like a string class or whatever
uint32_t hash_string(const std::string& str);

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

typedef struct { uint32_t datas[2]; } serialized;

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

// namespace anserial
}
