#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <immintrin.h>
#include "BitboardGenerator.h"
#include "Move.h"
#include "UndoHelper.h"

class ChessEngine {
public:
    enum PieceType {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE
    };

    enum Color {
        WHITE, BLACK
    };

    ChessEngine();

    uint64_t getAllPieces() const;

    std::string bitboardToString(const uint64_t bitboard) const;

    MoveList getMoves(const Color color) const;

    void makeMove(const Move move, const Color colorToMove);
    void undoMove(const Color colorThatMoved);

    unsigned long long perft(const int depth, const Color colorToMove);

private:
    PieceType squarePieceType[64]; // Array that stores the piece type of each square

    uint64_t pieces[2][6]; // Bitboards for all types of pieces of each color (first index for color, second index for piece type)
    uint64_t allPieces[2]; // Bitboards that hold all pieces of each color
    
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

    void initializeSquarePieceTypeArray(); // Initialize the array that stores piece type for every square with the classic chess setup

    void initializePawnMovesetBitboards(); // Initialize pawn push and attack bitboards
    void initializeKnightMovesetBitboards(); // Initialize knight moveset bitboards
    void initializeKingMovesetBitboards(); // Initialize king moveset bitboards

    void initializeRookOccupancyMasks(); // Initialize rook occupancy masks
    void initializeRookMovesetBitboards(); // Initialize rook moveset bitboards

    void initializeBishopOccupancyMasks(); // Initialize bishop occupancy masks
    void initializeBishopMovesetBitboards(); // Initialize bishop moveset bitboards

    void addPawnMoves(const Color color, MoveList& moveList, const uint64_t ownPieces, const uint64_t enemyPieces) const; // Add all the pawn moves of the given color to the move list
    void addKnightMoves(const Color color, MoveList& moveList, const uint64_t ownPieces) const; // Add all the knight moves of the given color to the move list
    void addKingMoves(const Color color, MoveList& movelist, const uint64_t ownPieces) const; // Add all the king moves of the given color to the move list
    void addRookMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const; // Add all the rook moves of the given color to the move list
    void addBishopMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const; // Add all the bishop moves of the given color to the move list
    void addQueenMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const; // Add all the queen moves of the given color to the move list

    std::stack<UndoHelper> undoStack; // Stack information about every move (for undo purposes)
};
