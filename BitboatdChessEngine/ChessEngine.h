#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <algorithm>
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

	struct SearchResult
	{
		Move move;
		int score;

		SearchResult() : move(Move()), score(0) {}
		SearchResult(const Move move, const int score) : move(move), score(score) {}
		SearchResult(const int score) : move(Move()), score(score) {}
	};

    ChessEngine(); // Chess engine constructor

    void loadFENPosition(const std::string position); // Load a chess position from a FEN string

    uint64_t getAllPieces() const; // Get a bitboard containing all pieces on the board

    std::string bitboardToString(const uint64_t bitboard) const; // Get a string representation of a bitboard
    std::string getSquareNotation(const int square) const; // Get the notation of a square (notation of square 0 is A1)

    MoveList getPseudolegalMoves(const Color color) const; // Get the pseudolegal moves of the given color
    MoveList getLegalMoves(const Color color); // Get the legal moves of the given color

    void makeMove(const Move move, const Color colorToMove); // Make a move on the board as the given color
    void undoMove(const Color colorThatMoved); // Undo the last move, color of the player that moved required

    unsigned long long perft(const int depth, const Color colorToMove); // Perft of a given depth starting with a given color

	SearchResult search(const int depth, const Color colorToMove); // Search for the best move of the given color by going to the given depth in the game tree

private:
    PieceType squarePieceType[64]; // Array that stores the piece type of each square

    uint64_t pieces[2][6]; // Bitboards for all types of pieces of each color (first index for color, second index for piece type)
    uint64_t allPieces[2]; // Bitboards that hold all pieces of each color

    uint8_t castlingRights; // The 4 least significant bits are used to store castling rights
    uint8_t whiteCastleQueenSide, whiteCastleKingSide, blackCastleQueenSide, blackCastleKingSide; // Values corresponding to each castling right

    uint64_t enPassantTargetBitboard; // Bitboard containing the squares that can be attacked by an "en passant" move

    uint64_t squaresBetween[64][64]; // Bitboards containing the squares between two other squares (if they are on the same line or diagonal)

    uint64_t pawnPushes[2][64], pawnAttacks[2][64]; // Bitboards for pawn movement
    uint64_t knightMovement[64]; // Bitboards for knight movement
    uint64_t kingMovement[64]; // Bitboards for king movement

    uint64_t rookOccupancyMask[64]; // Bitboards for rook occupancy masks
    uint64_t rookMovement[102400]; // Bitboards for rook movement
    int rookSquareOffset[64]; // Offset for each square in the rook movement array

    uint64_t bishopOccupancyMask[64]; // Bitboards for bishop occupancy masks
    uint64_t bishopMovement[5248]; // Bitboards for bishop movement
    int bishopSquareOffset[64]; // Offset for each square in the bishop movement array

    PieceType promotionPieceToPieceType[4]; // Get the corresponding piece type from an encoded promotion piece

    std::stack<UndoHelper> undoStack; // Stack information about every move (for undo purposes)

    void initializeBitboards(); // Initialize bitboards with the classic chess setup
    void initializeSquarePieceTypeArray(); // Initialize the array that stores piece type for every square with the classic chess setup
    void initializePromotionPieceToPieceTypeArray(); // Initialize the array that stores the corresponding piece type for every promotion type
    void initializePositionSpecialStatistics(); // Initialize castling rights, en passant squares, number of moves

    void initializeSquaresBetweenBitboards(); // Initialize the bitboards containing the squares between two other squares

    void initializePawnMovesetBitboards(); // Initialize pawn push and attack bitboards
    void initializeKnightMovesetBitboards(); // Initialize knight moveset bitboards
    void initializeKingMovesetBitboards(); // Initialize king moveset bitboards

    void initializeRookOccupancyMasks(); // Initialize rook occupancy masks
    void initializeRookMovesetBitboards(); // Initialize rook moveset bitboards

    void initializeBishopOccupancyMasks(); // Initialize bishop occupancy masks
    void initializeBishopMovesetBitboards(); // Initialize bishop moveset bitboards

    void addPawnMoves(const Color color, MoveList& moveList, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the pawn moves of the given color (within the mask) to the move list
    void addKnightMoves(const Color color, MoveList& moveList, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the knight moves of the given color (within the mask) to the move list
    void addKingMoves(const Color color, MoveList& movelist) const; // Add all the king moves of the given color to the move list
    void addRookMoves(const Color color, MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the rook moves of the given color (within the mask) to the move list
    void addBishopMoves(const Color color, MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the bishop moves of the given color (within the mask) to the move list
    void addQueenMoves(const Color color, MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the queen moves of the given color (within the mask) to the move list

    bool isAttacked(const int square, const Color color) const; // Returns true if the given square is attacked by the given color, false otherwise
	uint64_t getAttacksBitboard(const int square, const Color color) const; // Returns the number of attacks the given color has on the given square

	MoveList getPseudolegalMovesInCheck(const Color color, const uint64_t attackingSquares) const; // Get the pseudolegal moves of the given color when in check (checked by the attacking squares)

	bool compareMoves(const Move firstMove, const Move secondMove) const; // Compare two moves using MVV-LVA
	void sortMoves(MoveList& movelist) const; // Sort the move list using MVV-LVA

	int evaluate() const; // Compute an evaluation of the current state of the board. Positive values favour white, negative values favour black.
	
	SearchResult negamax(int alpha, int beta, const int depth, const Color colorToMove); // Negamax algorithm with alpha beta pruning
	SearchResult minimax(int alpha, int beta, const int depth, const Color colorToMove); // Minimax algorithm with alpha beta pruning
};

constexpr int CHECKMATE_SCORE[2] = { SHRT_MIN, SHRT_MAX };

constexpr int pieceValue[6] =
{
	100,	// Pawn
	320,	// Knight
	330,	// Bishop
	500,	// Rook
	900,	// Queen
	0   // King
};

// Table describing positional value of each piece type
constexpr int positionValue[7][64] =
{
	// Pawn value table (index 0)
	{
		 0,   0,   0,   0,   0,   0,   0,   0,
		 5,  10,  10, -20, -20,  10,  10,   5,
		 5,  -5, -10,   0,   0, -10,  -5,   5,
		 0,   0,   0,  20,  20,   0,   0,   0,
		 5,   5,  10,  25,  25,  10,   5,   5,
		10,  10,  20,  30,  30,  20,  10,  10,
		50,  50,  50,  50,  50,  50,  50,  50,
		 0,   0,   0,   0,   0,   0,   0,   0
	},

	// Knight value table (index 1)
	{
		-50, -40, -30, -30, -30, -30, -40, -50,
		-40, -20,   0,   0,   0,   0, -20, -40,
		-30,   0,  10,  15,  15,  10,   0, -30,
		-30,   5,  15,  20,  20,  15,   5, -30,
		-30,   0,  15,  20,  20,  15,   0, -30,
		-30,   5,  10,  15,  15,  10,   5, -30,
		-40, -20,   0,   5,   5,   0, -20, -40,
		-50, -40, -30, -30, -30, -30, -40, -50
	},

	// Bishop value table (index 2)
	{
		-20, -10, -10, -10, -10, -10, -10, -20,
		-10,   5,   0,   0,   0,   0,   5, -10,
		-10,  10,  10,  10,  10,  10,  10, -10,
		-10,   0,  10,  10,  10,  10,   0, -10,
		-10,   5,   5,  10,  10,   5,   5, -10,
		-10,   0,   5,  10,  10,   5,   0, -10,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-20, -10, -10, -10, -10, -10, -10, -20
	},

	// Rook value table (index 3)
	{
		 0,   0,   0,   5,   5,   0,   0,   0,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		-5,   0,   0,   0,   0,   0,   0,  -5,
		 5,  10,  10,  10,  10,  10,  10,   5,
		 0,   0,   0,   0,   0,   0,   0,   0
	},

	// Queen value table (index 4)
	{
		-20, -10, -10,  -5,  -5, -10, -10, -20,
		-10,   0,   0,   0,   0,   0,   0, -10,
		-10,   0,   5,   5,   5,   5,   0, -10,
		  0,   0,   5,   5,   5,   5,   0,  -5,
		 -5,   0,   5,   5,   5,   5,   0,  -5,
		-10,   0,   5,   5,   5,   5,   0, -10,
		-10,   0,   5,   0,   0,   0,   0, -10,
		-20, -10, -10,  -5,  -5, -10, -10, -20
	},

	// King early and mid game value table (index 5)
	{
		 20,  20,   0,   0,   0,   0,  20,  20,
		 20,  30,  10,   0,   0,  10,  30,  20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30
	},

	// King end game value table (index 6)
	{
		-50, -40, -30, -20, -20, -30, -40, -50,
		-30, -20, -10,   0,   0, -10, -20, -30,
		-30, -10,  20,  30,  30,  20, -10, -30,
		-30, -10,  30,  40,  40,  30, -10, -30,
		-30, -10,  30,  40,  40,  30, -10, -30,
		-30, -10,  20,  30,  30,  20, -10, -30,
		-30, -30,   0,   0,   0,   0, -30, -30,
		-50, -30, -30, -30, -30, -30, -30, -50
	}
};