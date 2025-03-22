#include "Move.h"

Move::Move() : moveData(0) {}

Move::Move(const std::string moveString)
{
	// Check string size
	if (moveString.size() < 4 || moveString.size() > 5)
		throw std::invalid_argument("The given string does not resprect UCI rules (String too short or too long).");

	// Check files
	if (!('a' <= moveString[0] && moveString[0] <= 'h') || !('a' <= moveString[2] && moveString[2] <= 'h'))
		throw std::invalid_argument("The given string does not resprect UCI rules (Files not represented correctly).");

	// Check ranks
	if (!('1' <= moveString[1] && moveString[1] <= '9') || !('1' <= moveString[3] && moveString[3] <= '9'))
		throw std::invalid_argument("The given string does not resprect UCI rules (Ranks not represented correctly).");

	// Check promotion type
	if (moveString.size() == 5 && (moveString[4] != 'q' && moveString[4] != 'n' && moveString[4] != 'r' && moveString[4] != 'b'))
		throw std::invalid_argument("The given string does not resprect UCI rules (Promotion type not represented correctly).");

	int fromRank = moveString[1] - '1';
	int fromFile = moveString[0] - 'a';
	int fromSquare = 8 * fromRank + fromFile;

	int toRank = moveString[3] - '1';
	int toFile = moveString[2] - 'a';
	int toSquare = 8 * toRank + toFile;

	if (moveString.size() == 4) // Normal move
	{
		moveData = (fromSquare & 0x3F) |           // 6 bits for 'from' square
			((toSquare & 0x3F) << 6) |             // 6 bits for 'to' square
			((MoveType::NORMAL & 0x3) << 12) |    // 2 bits for special move type
			((PromotionPiece::KNIGHT & 0x3) << 14);      // 2 bits for promotion piece
	}
	else if (moveString.size() == 5) // Promotion move
	{
		PromotionPiece promotion = PromotionPiece::QUEEN;
		switch (moveString[4])
		{
		case 'q':
			promotion = PromotionPiece::QUEEN;
			break;
		case 'n':
			promotion = PromotionPiece::KNIGHT;
			break;
		case 'r':
			promotion = PromotionPiece::ROOK;
			break;
		case 'b':
			promotion = PromotionPiece::BISHOP;
			break;
		}

		moveData = (fromSquare & 0x3F) |           // 6 bits for 'from' square
			((toSquare & 0x3F) << 6) |             // 6 bits for 'to' square
			((MoveType::PROMOTION & 0x3) << 12) |    // 2 bits for special move type
			((promotion & 0x3) << 14);      // 2 bits for promotion piece
	}
}

std::string getSquareNotation(const int square)
{
	if (square < 0 || square > 63)
		return "Invalid square";

	char file = 'a' + (square % 8); // Get file (column) from 'a' to 'h'
	int rank = 1 + (square / 8);    // Get rank (row) from 1 to 8

	return std::string(1, file) + std::to_string(rank);
}

std::string Move::toString() const
{
	std::string moveString = getSquareNotation(this->from())+ getSquareNotation(this->to());

	if (moveType() == MoveType::PROMOTION)
	{
		PromotionPiece piece = promotionPiece();
		switch (piece)
		{
		case PromotionPiece::QUEEN:
			moveString += "q";
			break;
		case PromotionPiece::KNIGHT:
			moveString += "n";
			break;
		case PromotionPiece::ROOK:
			moveString += "r";
			break;
		case PromotionPiece::BISHOP:
			moveString += "b";
			break;
		}
	}

	return moveString;
}

bool Move::operator==(const Move& other) const
{
	return this->moveData == other.moveData;
}

MoveList::MoveList() : numberOfMoves(0) {}
