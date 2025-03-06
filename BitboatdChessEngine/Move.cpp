#include "Move.h"

std::string Move::toString() const
{
	return std::to_string(this->from()) + " -> " + std::to_string(this->to());
}
