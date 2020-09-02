#include <stdexcept>
#include <string>
#include <iostream>

#include "ActualPlayer.hpp"


int main(int argc, char** argv) {
	try {
		// if (argc != 2) {
		// 	throw std::logic_error{"Not enough arguments"};
		// }

		ActualPlayer play{argv[1]};
		play();
	}

	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what();
		return -1;
	}

	return 0;
}
