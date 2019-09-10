#include <anserial/anserial.hpp>

using namespace anserial;

int main(int argc, char *argv[]) {
	deserializer der;

	// TODO: we should make some deserializer functions that can handle
	//       input from file streams
	uint32_t buf[1024];
	size_t nred = 0;

	while ((nred = fread(buf, 8, 512, stdin)) > 0) {
		der.deserialize(buf, nred);
	}

	s_tree foo(&der);
	s_node *results = nullptr;

	if (!destructure(foo.data(), {&results})) {
		printf("; couldn't find results container...\n");
		return 1;
	}

	for (s_node *node : results->entities()) {
		uint32_t i_19937, i_2048;

		if (destructure(node,
			{"results",
				{"i-19937", &i_19937},
				{"i-2048", &i_2048}}))
		{
			printf("; have result: %u and %u\n", i_19937, i_2048);

		} else {
			printf("; invalid result structure, continuing...\n");
		}
	}

	return 0;
}
