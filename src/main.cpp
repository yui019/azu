#include "azu.h"

int main() {
	auto context = azu::Context("Title", 800, 600);
	azu::run(context);

	return 0;
}