#pragma once
#include <cstdint>
#include "Move.h"

class UndoHelper
{
private:
	uint32_t moveData;

public:
	// Default constructor
	UndoHelper();

	// Constructor
	UndoHelper(const int from, const int to, const uint8_t castlingRights, const Move::MoveType moveType = Move::MoveType::NORMAL, const int capturedPieceType = 6)
	{
		moveData = (from & 0x3F) |				// 6 bits for 'from' square
			((to & 0x3F) << 6) |				// 6 bits for 'to' square
			((castlingRights & 0xF) << 12) |	// 4 bits for castlingRights
			((moveType & 0x3) << 16) |          // 2 bits for special move type
			((capturedPieceType & 0x7) << 18);  // 3 bits for captured piece type
	}

	// The position from which the piece moves
	inline int from() const { return moveData & 0x3F; }
	// The position the piece moves to
	inline int to() const { return (moveData >> 6) & 0x3F; }
	// The castling rights
	inline uint8_t castlingRights() const { return (moveData >> 12) & 0xF; }
	// The type of the move (normal, castle, en passant, promotion)
	inline Move::MoveType moveType() const { return static_cast<Move::MoveType>((moveData >> 16) & 0x3); }
	// The type of the captured piece
	inline int capturedPieceType() const { return (moveData >> 18) & 0x7; }
};
