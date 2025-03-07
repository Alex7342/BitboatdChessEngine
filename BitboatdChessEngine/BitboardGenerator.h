#pragma once
#include <cstdint>

class BitboardGenerator
{
public:
	static const uint64_t RANK_1 = 0x00000000000000FF;   // Rank 1 (bits 0-7)
	static const uint64_t RANK_2 = 0x000000000000FF00;   // Rank 2 (bits 8-15)
	static const uint64_t RANK_3 = 0x0000000000FF0000;   // Rank 3 (bits 16-23)
	static const uint64_t RANK_4 = 0x00000000FF000000;   // Rank 4 (bits 24-31)
	static const uint64_t RANK_5 = 0x000000FF00000000;   // Rank 5 (bits 32-39)
	static const uint64_t RANK_6 = 0x0000FF0000000000;   // Rank 6 (bits 40-47)
	static const uint64_t RANK_7 = 0x00FF000000000000;   // Rank 7 (bits 48-55)
	static const uint64_t RANK_8 = 0xFF00000000000000;   // Rank 8 (bits 56-63)

	static const uint64_t FILE_A = 0x0101010101010101;   // File A (bits 0, 8, 16, ..., 56)
	static const uint64_t FILE_B = 0x0202020202020202;   // File B (bits 1, 9, 17, ..., 57)
	static const uint64_t FILE_C = 0x0404040404040404;   // File C (bits 2, 10, 18, ..., 58)
	static const uint64_t FILE_D = 0x0808080808080808;   // File D (bits 3, 11, 19, ..., 59)
	static const uint64_t FILE_E = 0x1010101010101010;   // File E (bits 4, 12, 20, ..., 60)
	static const uint64_t FILE_F = 0x2020202020202020;   // File F (bits 5, 13, 21, ..., 61)
	static const uint64_t FILE_G = 0x4040404040404040;   // File G (bits 6, 14, 22, ..., 62)
	static const uint64_t FILE_H = 0x8080808080808080;   // File H (bits 7, 15, 23, ..., 63)

	static inline uint64_t north(const uint64_t square) { return (square << 8); } // Shift square 8 bits left to move north
	static inline uint64_t south(const uint64_t square) { return (square >> 8); } // Shift square 8 bits right to move south
	static inline uint64_t east(const uint64_t square) { return (square << 1) & ~FILE_A; } // Shift square 1 bit left to move east, mask to avoid wrapping around the left edge
	static inline uint64_t west(const uint64_t square) { return (square >> 1) & ~FILE_H; } // Shift square 1 bit right to move west, mask to avoid wrapping around the right edge

	static inline uint64_t northEast(const uint64_t square) { return (square << 9) & ~FILE_A; } // Shift square 9 bits left (north-east), mask to avoid wrapping around the left edge
	static inline uint64_t northWest(const uint64_t square) { return (square << 7) & ~FILE_H; } // Shift square 7 bits left (north-west), mask to avoid wrapping around the right edge
	static inline uint64_t southEast(const uint64_t square) { return (square >> 7) & ~FILE_A; } // Shift square 7 bits right (south-east), mask to avoid wrapping around the left edge
	static inline uint64_t southWest(const uint64_t square) { return (square >> 9) & ~FILE_H; } // Shift square 9 bits right (south-west), mask to avoid wrapping around the right edge

	// Generate a bitboard containing the square the pawn on the given square and color (white = 0, black = 1) can push to
	static uint64_t generatePawnPush(const uint64_t pawnBitboard, const int color);
	// Generate a bitboard containing the attacked squares by the pawn on the given square and of the given color (white = 0, black = 1)
	static uint64_t generatePawnAttack(const uint64_t pawnBitboard, const int color);

	// Generate a bitboard containing knight moves
	static uint64_t generateKnightMoveset(const uint64_t knightBitboard);
};
