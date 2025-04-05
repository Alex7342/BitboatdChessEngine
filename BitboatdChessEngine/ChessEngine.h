#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>
#include <immintrin.h>
#include "BitboardGenerator.h"
#include "Move.h"
#include "UndoHelper.h"

constexpr int MAX_DEPTH = 64;
constexpr int MAX_SEE_DEPTH = 16;

constexpr int NULL_MOVE_DEPTH_THRESHOLD = 4;
constexpr int NULL_MOVE_DEPTH_REDUCTION = 2;

class ChessEngine {
public:
	enum PieceType {
		PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE
	};

	enum Color {
		WHITE, BLACK
	};

	enum NodeType {
		EXACT, LOWER_BOUND, UPPER_BOUND
	};

	struct TranspositionTableEntry
	{
		uint64_t zobristHash;
		Move move;
		int score;
		int depth;
		NodeType nodeType;

		TranspositionTableEntry() : zobristHash(0ULL), move(Move()), score(0), depth(0), nodeType(NodeType::EXACT) {}
		TranspositionTableEntry(const uint64_t zobrishHash, const Move move, const int score, const int depth, const NodeType nodeType) : zobristHash(zobrishHash), move(move), score(score), depth(depth), nodeType(nodeType) {}
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
	~ChessEngine(); // Chess engine destructor

	void loadFENPosition(const std::string position); // Load a chess position from a FEN string

	uint64_t getAllPieces() const; // Get a bitboard containing all pieces on the board

	std::string bitboardToString(const uint64_t bitboard) const; // Get a string representation of a bitboard
	std::string getSquareNotation(const int square) const; // Get the notation of a square (notation of square 0 is A1)

	Move getMoveFromString(const std::string moveString) const;
	MoveList getPseudolegalMoves() const; // Get the pseudolegal moves of the active player
	MoveList getLegalMoves(); // Get the legal moves of the active player

	void makeMove(const Move move); // Make a move on the board
	void undoMove(); // Undo the last move

	unsigned long long perft(const int depth); // Perft of a given depth
	SearchResult getBestMove(); // Get the best move in the current position

	void stopCurrentSearch(); // Stop the current search
	void clearTranspositionTable(); // Clear the transposition table
	void clearMoveOrderingTables(); // Clear move ordering tables

	void setWhiteTime(const int timeInMilliseconds); // Set the remaining time of white (in milliseconds)
	void setBlackTime(const int timeInMilliseconds); // Set the remaining time of black (in milliseconds)
	void setWhiteIncrement(const int incrementInMilliseconds); // Set the increment per move of white
	void setBlackIncrement(const int incrementInMilliseconds); // Set the increment per move of black

	uint64_t getZobristHash() const; // Get the zobrist hash for the current state of the board

private:
	Color activePlayer; // The currently active player
	int halfmoveClock; // The halfmove clock
	int fullmoveCounter; // The fullmove counter
	
	int previousPositionsSize;
	uint64_t* previousPositions; // Zobrist hashes for previous positions

	int timeRemaining[2]; // Time remaining in milliseconds for each player
	int timeIncrement[2]; // Time increment in millisecond after each move for each player
	void initializeTimeLimits(); // Initialize the remaining time and the time increment arrays

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
	uint64_t* rookMovement; // Bitboards for rook movement
	int rookSquareOffset[64]; // Offset for each square in the rook movement array

	uint64_t bishopOccupancyMask[64]; // Bitboards for bishop occupancy masks
	uint64_t* bishopMovement; // Bitboards for bishop movement
	int bishopSquareOffset[64]; // Offset for each square in the bishop movement array

	PieceType promotionPieceToPieceType[4]; // Get the corresponding piece type from an encoded promotion piece

	uint64_t pieceZobristHash[6][64]; // Zobrist hash for each piece type on every square of the board
	uint64_t castlingRightsZobristHash[16]; // Zobrist hash for each possible combination of castling rights
	uint64_t enPassantTargetSquareZobristHash[64]; // Zobrist hash for each en passant target square
	uint64_t changePlayerZobristHash; // Zobrist hash for changing the active player
	uint64_t boardZobristHash; // The zobrist hash for the current state of the board

	const int transpositionTableSize = 1 << 25; // The size of the transposition table
	TranspositionTableEntry* transpositionTable; // Transposition table

	int historyTable[2][64][64]; // Table used for history heuristic
	bool maxHistoryValueReached; // Flag set when the max history value is reached
	void decayHistoryTable(); // Scale down the values in the history heuristic (used to avoid overflow)
	void updateHistoryTable(const Color color, const Move move, const int depth); // Update the history table for the given color, move and depth

	Move killerMoves[MAX_DEPTH][2]; // Table that stores killer moves by ply
	void updateKillerMoves(const Move move, const int ply); // Update the killer moves table
	void clearKillerMoves(); // Clear the killer moves table

	uint64_t getXrayAttacksToSquare(const int square, const Color color) const; // Get all x-ray attacks of the given color to the given square
	int SEE(const int square, const Color color) const; // Static exchange evaluation for the given square and color
	int recursiveSEE(const int square, const Color color) const; // Recursive static exchange evaluation for the given square and color

    std::stack<UndoHelper> undoStack; // Stack information about every move (for undo purposes)

    void initializeBitboards(); // Initialize bitboards with the classic chess setup
    void initializeSquarePieceTypeArray(); // Initialize the array that stores piece type for every square with the classic chess setup
    void initializePromotionPieceToPieceTypeArray(); // Initialize the array that stores the corresponding piece type for every promotion type
    void initializePositionSpecialStatistics(); // Initialize castling rights, en passant squares, number of moves
	void initializeZobristHash(); // Initialize zobrist hashes
	void initializeMoveOrderingTables(); // Initialize the tables used for move ordering

    void initializeSquaresBetweenBitboards(); // Initialize the bitboards containing the squares between two other squares

    void initializePawnMovesetBitboards(); // Initialize pawn push and attack bitboards
    void initializeKnightMovesetBitboards(); // Initialize knight moveset bitboards
    void initializeKingMovesetBitboards(); // Initialize king moveset bitboards

    void initializeRookOccupancyMasks(); // Initialize rook occupancy masks
    void initializeRookMovesetBitboards(); // Initialize rook moveset bitboards

    void initializeBishopOccupancyMasks(); // Initialize bishop occupancy masks
    void initializeBishopMovesetBitboards(); // Initialize bishop moveset bitboards

    void addPawnMoves(MoveList& moveList, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the pawn moves of the active player (within the mask) to the move list
    void addKnightMoves(MoveList& moveList, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the knight moves of the active player (within the mask) to the move list
    void addKingMoves(MoveList& movelist) const; // Add all the king moves of the active player to the move list
    void addRookMoves(MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the rook moves of the active player (within the mask) to the move list
    void addBishopMoves(MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the bishop moves of the active player (within the mask) to the move list
    void addQueenMoves(MoveList& movelist, const uint64_t mask = 0xFFFFFFFFFFFFFFFF) const; // Add all the queen moves of the active player (within the mask) to the move list

    bool isAttacked(const int square, const Color color) const; // Returns true if the given square is attacked by the given color, false otherwise
	uint64_t getAttacksBitboard(const int square, const Color color) const; // Returns the number of attacks the given color has on the given square

	MoveList getPseudolegalMovesInCheck(const uint64_t attackingSquares) const; // Get the pseudolegal moves of the active player when in check (checked by the attacking squares)

	bool isValid(const Move move); // Check if the given move is valid in the current state of the board

	bool compareMoves(const Move firstMove, const Move secondMove) const; // Compare two moves using MVV-LVA
	void sortMoves(MoveList& movelist) const; // Sort the move list using MVV-LVA
	int assignScore(const Move move) const;

	int evaluate() const; // Compute an evaluation of the current state of the board. Positive values favour white, negative values favour black.
	
	int currentPly; // The ply the search is currently at
	bool isAtRoot; // True if the search is at root level, false otherwise
	int numberOfNodesVisited;
	SearchResult minimax(int alpha, int beta, const int depth, const int ply); // Minimax algorithm with alpha beta pruning
	int getTimeForSearch() const;

	SearchResult search(const int depth); // Search for the best move of the active player by going to the given depth in the game tree
	SearchResult iterativeDeepeningSearch(const int timeLimit); // Ssearch for the best move of the active player within the time limit (in milliseconds)
	std::chrono::steady_clock::time_point searchStartTime; // The time the search started
	int timeLimitInMilliseconds; // The time allocated to the search in milliseconds
	bool stopSearch; // Flag set to true when the time limit is exceeded
};

constexpr int CHECKMATE_SCORE[2] = { SHRT_MIN, SHRT_MAX };

constexpr int pieceValue[7] =
{
	100,	// Pawn
	320,	// Knight
	330,	// Bishop
	500,	// Rook
	900,	// Queen
	0,		// King
	0		// None
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

constexpr int MAX_HISTORY_VALUE = 1 << 29;
