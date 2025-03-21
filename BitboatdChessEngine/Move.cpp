#include "Move.h"

Move::Move() : moveData(0) {}

std::string Move::toString() const
{
	std::string moveString = std::to_string(this->from()) + " -> " + std::to_string(this->to());

	if (moveType() == MoveType::PROMOTION)
	{
		PromotionPiece piece = promotionPiece();
		switch (piece)
		{
		case PromotionPiece::QUEEN:
			moveString += " Q";
			break;
		case PromotionPiece::KNIGHT:
			moveString += " N";
			break;
		case PromotionPiece::ROOK:
			moveString += " R";
			break;
		case PromotionPiece::BISHOP:
			moveString += " B";
			break;
		}
	}

	return moveString;
}

MoveList::MoveList() : numberOfMoves(0) {}
