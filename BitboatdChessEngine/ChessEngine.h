#pragma once
#include <cstdint>
#include <string>
#include "BitboardGenerator.h"

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
    
    uint64_t pawnPushes[2][64], pawnAttacks[2][64]; // Bitboards for pawn movement
    uint64_t knightMovement[64]; // Bitboards for knight movement
    uint64_t kingMovement[64]; // Bitboards for king movement
    uint64_t slidingPiecesMovement[]; // Bitboards for sliding pieces movement

    void initializeBitboards(); // Initialize bitboards with the classic chess setup

    void initializePawnMovesetBitboards(); // Initialize pawn push and attack bitboards
    void initializeKnightMovesetBitboards(); // Initialize knight moveset bitboards
    void initializeKingMovesetBitboards(); // Initialize king moveset bitboards
};
