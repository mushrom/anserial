#include <anserial/s_tree.hpp>
#include <iostream>

using namespace anserial;

s_tree::s_tree() {

}

s_tree::s_tree(s_node *node) {
	top = node;
	refresh();
}

s_tree::s_tree(deserializer* nder) {
	der = nder;
	refresh();
}

s_node *s_tree::lookup(std::string& symbol) {
	return lookup(hash_string(symbol));
}

s_node *s_tree::lookup(uint32_t hash) {
	if (symtab) {
		return symtab->get(hash);
	}

	return nullptr;
}

void s_tree::refresh(void) {
	if (!top && der) {
		top = der->deserialize();
	}

	if (top && top->self.d_type == ENT_TYPE_MAP) {
		symtab = top->get("::symtab");
		version = top->get("::version");
		data = top->get("::data");
	}
}

void s_tree::dump_nodes(void) {
	dump_nodes(top, 0);
}

void s_tree::dump_nodes(s_node *node, unsigned indent) {
	if (!node) {
		printf("#<nullptr>");
		return;
	}

	const char *types[] = {
		"container", "symbol", "integer", "string",
		"map", "set", "null"
	};

	//printf("%*s(%s", 4*indent, " ", types[node->self.d_type]);
	printf("%*s", 4*indent, " ");

	// TODO: we should use the node type here, rather than the
	//       type from the raw deserialized entity...
	switch (node->self.d_type) {
		case ENT_TYPE_CONTAINER:
			printf("(");
			for (auto& x : node->entities()) {
				if (x->self.id != node->self.id) {
					putchar('\n');
					dump_nodes(x, indent + 1);
				}
			}
			putchar(')');

			break;

		case ENT_TYPE_MAP:
			printf("(map");
			for (auto& x : node->keys()) {
				putchar('\n');

				dump_nodes(x, indent + 1);
				dump_nodes(node->get(x->uint()), indent + 1);
			}
			putchar(')');

			break;

		case ENT_TYPE_STRING:
			std::cout << '"' << node->string() << '"';
			break;

		case ENT_TYPE_SYMBOL:
			{
				s_node *str = lookup(node->uint());

				if (str) {
					std::cout << str->string() << ' ';

				} else {
					printf("#<symbol:#x%x>", node->uint());

				}
				//printf("#<symbol:#x%x>", node->uint());
			}
			break;

		case ENT_TYPE_INTEGER:
			printf("%u", node->uint());
			break;
	}

	if (indent == 0) {
		putchar('\n');
	}
}
