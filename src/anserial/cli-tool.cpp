#include <anserial/anserial.hpp>

using namespace anserial;

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
