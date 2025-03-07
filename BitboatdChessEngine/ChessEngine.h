#pragma once
#include <cstdint>
#include <string>
#include <array>

class ChessEngine {
public:
    enum PieceType {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE
    };

    enum Color {
        WHITE, BLACK
    };

    ChessEngine();
    std::string bitboardToString(const uint64_t bitboard) const;

private:
    uint64_t pieces[2][6]; // Bitboards for all types of pieces of each color (first index for color, second index for piece type)

    void initializeBitboards(); // Initialize bitboards with the classic chess setup
};
