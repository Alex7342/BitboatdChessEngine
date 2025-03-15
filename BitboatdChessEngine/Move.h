#pragma once
#include <cstdint>
#include <string>

class Move {
private:
    uint16_t moveData;  // Bit-packed move representation

public:
    // Explicitly-sized enums to ensure compact storage
    enum MoveType : uint8_t {
        NORMAL = 0,
        CASTLE = 1,
        EN_PASSANT = 2,
        PROMOTION = 3
    };

    enum PromotionPiece : uint8_t {
        KNIGHT = 0,
        BISHOP = 1,
        ROOK = 2,
        QUEEN = 3
    };

    // Default constructor
    Move();

    // Constructor
    Move(const int from, const int to, const MoveType moveType = NORMAL, const PromotionPiece promoPiece = KNIGHT) {
        moveData = (from & 0x3F) |           // 6 bits for 'from' square
            ((to & 0x3F) << 6) |             // 6 bits for 'to' square
            ((moveType & 0x3) << 12) |    // 2 bits for special move type
            ((promoPiece & 0x3) << 14);      // 2 bits for promotion piece
    }

    // The position from which the piece moves
    inline int from() const { return moveData & 0x3F; }
    // The position the piece moves to
    inline int to() const { return (moveData >> 6) & 0x3F; }
    // The type of the move (normal, castle, en passant, promotion)
    inline MoveType moveType() const { return static_cast<MoveType>((moveData >> 12) & 0x3); }
    // The type of piece to promote to (in case of pawn promotion)
    inline PromotionPiece promotionPiece() const { return static_cast<PromotionPiece>((moveData >> 14) & 0x3); }

    // Get raw move data
    inline uint16_t raw() const { return moveData; }

    std::string toString() const;
};

struct MoveList
{
    Move moves[256];
    short numberOfMoves;

    inline void add(const Move move) { moves[numberOfMoves++] = move; }

    MoveList();
};