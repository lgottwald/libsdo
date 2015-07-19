#include "RandomUniform.hpp"

namespace sdo { namespace random {

std::random_device rd;
std::mt19937 gen(rd());

}}
