#include "Move.h"

Move::Move() : moveData(0) {}

std::string Move::toString() const
{
	return std::to_string(this->from()) + " -> " + std::to_string(this->to());
}

MoveList::MoveList() : numberOfMoves(0) {}
