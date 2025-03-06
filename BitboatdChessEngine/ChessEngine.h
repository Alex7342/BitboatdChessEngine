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
    std::array<uint64_t, 6> whitePieces; // Bitboards for white pieces
    std::array<uint64_t, 6> blackPieces; // Bitboards for black pieces

    void initializeBitboards(); // Initialize bitboards with the classic chess setup
};
