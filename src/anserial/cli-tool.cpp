#include <anserial/anserial.hpp>

using namespace anserial;

void gen_test_data(void) {
	serializer foo;

	uint32_t top = foo.default_layout();

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

	foo.add_symtab(0);

	auto buf = foo.serialize();
	fwrite(buf.data(), 4, buf.size(), stdout);
}

void decode_dump(void) {
	deserializer der;

	uint32_t buf[1024];
	size_t nred = 0;

	while ((nred = fread(buf, 8, 512, stdin)) > 0) {
		der.deserialize(buf, nred);
	}

	s_tree foo(&der);
	foo.dump_nodes();
}

void print_help(void) {
	puts(
		" -h : print this help and exit\n"
		" -d : decode and dump serialized data from stdin\n"
		" -e : serialize s-expressions from stdin"
		" -t : generate some test data"
	);
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		switch (argv[1][1]) {
			case 'h':
				print_help();
				return 0;
			case 'd':
				decode_dump();
				return 0;
			case 'e':
				puts("TODO: not implemented");
				return 1;
			case 't':
				gen_test_data();
				return 0;
			default:
				puts("invalid option!");
				print_help();
				return 1;
		}
	} else {
		print_help();
		return 0;
	}

	return 0;
}
