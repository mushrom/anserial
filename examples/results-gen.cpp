#include <anserial/anserial.hpp>

using namespace anserial;

int main(int argc, char *argv[]) {
	int entries = (argc >= 2)? atoi(argv[1]) : 100;

	serializer foo;
	uint32_t top = foo.default_layout();

	uint32_t counting = foo.add_entities(top, {});
	for (int i = 0; i < entries; i++) {
		foo.add_entities(counting,
			{"results",
				{"i-19937", (unsigned)i*19937},
				{"i-2048",  (unsigned)i*2048}});
	}

	foo.add_symtab(0);

	auto buf = foo.serialize();
	fwrite(buf.data(), 4, buf.size(), stdout);

	return 0;
}
