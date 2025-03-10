#pragma once
#include <cstdint>
#include <string>
#include <immintrin.h>
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

    uint64_t rookOccupancyMask[64]; // Bitboards for rook occupancy masks
    uint64_t rookMovement[102400]; // Bitboards for rook movement
    int rookSquareOffset[64]; // Offset for each square in the rook movement array

    uint64_t bishopOccupancyMask[64]; // Bitboards for bishop occupancy masks
    uint64_t bishopMovement[5248]; // Bitboards for bishop movement
    int bishopSquareOffset[64]; // Offset for each square in the bishop movement array

    void initializeBitboards(); // Initialize bitboards with the classic chess setup

    void initializePawnMovesetBitboards(); // Initialize pawn push and attack bitboards
    void initializeKnightMovesetBitboards(); // Initialize knight moveset bitboards
    void initializeKingMovesetBitboards(); // Initialize king moveset bitboards

    void initializeRookOccupancyMasks(); // Initialize rook occupancy masks
    void initializeRookMovesetBitboards(); // Initialize rook moveset bitboards

    void initializeBishopOccupancyMasks(); // Initialize bishop occupancy masks
    void initializeBishopMovesetBitboards(); // Initialize bishop moveset bitboards
};
