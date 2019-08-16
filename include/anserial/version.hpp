#pragma once
#include <stdint.h>

namespace anserial {

// semantic versioning, just in case
struct { uint32_t major, minor, patch; } version = {0, 0, 1};

}
