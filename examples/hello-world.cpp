#include <anserial/anserial.hpp>

using namespace anserial;

int main(int argc, char *argv[]) {
	serializer foo;
	uint32_t top = foo.default_layout();

	foo.add_entities(top,
		{"Hello", "world!",
			{1, 2, 3}});

	foo.add_symtab(0);

	auto buf = foo.serialize();
	fwrite(buf.data(), 4, buf.size(), stdout);

	return 0;
}
