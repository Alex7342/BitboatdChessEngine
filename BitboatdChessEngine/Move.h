#pragma once
#include <cstdint>
#include <string>

class Move {
private:
    uint16_t moveData;  // Bit-packed move representation

public:
    // Use explicitly-sized enums to ensure compact storage
    enum SpecialMove : uint8_t {
        NORMAL = 0,
        CASTLING = 1,
        EN_PASSANT = 2,
        PROMOTION = 3
    };

    enum PromotionPiece : uint8_t {
        KNIGHT = 0,
        BISHOP = 1,
        ROOK = 2,
        QUEEN = 3
    };

    // Constructor
    Move(int from, int to, SpecialMove specialMove = NORMAL, PromotionPiece promoPiece = KNIGHT) {
        moveData = (from & 0x3F) |           // 6 bits for 'from' square
            ((to & 0x3F) << 6) |             // 6 bits for 'to' square
            ((specialMove & 0x3) << 12) |    // 2 bits for special move type
            ((promoPiece & 0x3) << 14);      // 2 bits for promotion piece
    }

    // Inline getters using bitwise operations
    inline int from() const { return moveData & 0x3F; }
    inline int to() const { return (moveData >> 6) & 0x3F; }
    inline SpecialMove specialMove() const { return static_cast<SpecialMove>((moveData >> 12) & 0x3); }
    inline PromotionPiece promotionPiece() const { return static_cast<PromotionPiece>((moveData >> 14) & 0x3); }

    inline uint16_t raw() const { return moveData; } // Get raw move data

    std::string toString() const;
};