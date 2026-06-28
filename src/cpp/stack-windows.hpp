#pragma once

#include <fstream>
#include <addon-tools.hpp>


namespace segfault {
	DBG_EXPORT void showCallstack(std::ofstream &outfile);
}
