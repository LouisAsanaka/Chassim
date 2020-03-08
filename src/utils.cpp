#include "utils.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>

double ROUND1(double x) {
	return (std::ceil(x * 10.0) / 10.0);
}

double ROUND2(double x) { 
	return (std::ceil(x * 100.0) / 100.0); 
}

std::string ROUND1STR(double x) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(1) << x;
	return stream.str();
}

std::string ROUND2STR(double x) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(2) << x;
	return stream.str();
}