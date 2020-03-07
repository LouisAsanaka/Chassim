#include "utils.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>

// Default precision to 2 places
double ROUND(double x) { 
	return (std::ceil(x * 100.0) / 100.0); 
}

std::string ROUND2STR(double x) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(2) << x;
	return stream.str();
}