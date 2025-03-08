#include "BitboardGenerator.h"

uint64_t BitboardGenerator::generatePawnPush(const uint64_t pawnBitboard, const int color)
{
	if (color == 0)
	{
		return north(pawnBitboard);
	}
	else if (color == 1)
	{
		return south(pawnBitboard);
	}

	return 0;
}

uint64_t BitboardGenerator::generatePawnAttack(const uint64_t pawnBitboard, const int color)
{
	if (color == 0)
	{
		return northWest(pawnBitboard) | northEast(pawnBitboard);
	}
	else if (color == 1)
	{
		return southWest(pawnBitboard) | southEast(pawnBitboard);
	}

	return 0;
}

uint64_t BitboardGenerator::generateKnightMoveset(const uint64_t knightBitboard)
{
	uint64_t movePattern =
		north(west(west(knightBitboard))) |   // 1 square up, 2 squares left
		west(north(north(knightBitboard))) |  // 2 squares up, 1 square left
		east(north(north(knightBitboard))) |  // 2 squares up, 1 square right
		north(east(east(knightBitboard))) |   // 1 square up, 2 squares right
		south(east(east(knightBitboard))) |   // 1 square down, 2 squares right
		east(south(south(knightBitboard))) |  // 2 squares down, 1 square right
		west(south(south(knightBitboard))) |  // 2 squares down, 1 square left
		south(west(west(knightBitboard)));    // 1 square down, 2 squares left

	return movePattern;
}

uint64_t BitboardGenerator::generateKingMoveset(const uint64_t kingBitboard)
{
	uint64_t movePattern =
		north(west(kingBitboard)) |
		north(kingBitboard) |
		north(east(kingBitboard)) |
		east(kingBitboard) |
		south(east(kingBitboard)) |
		south(kingBitboard) |
		south(west(kingBitboard)) |
		west(kingBitboard);

	return movePattern;
}

uint64_t BitboardGenerator::generateRookOccupancyMask(const uint64_t rookBitboard)
{
	uint64_t occupancyMask = 0;

	// Directions of rook movement
	uint64_t north = rookBitboard;
	uint64_t south = rookBitboard;
	uint64_t east = rookBitboard;
	uint64_t west = rookBitboard;

	// Maximum 6 bits in each direction
	for (int i = 0; i < 6; i++)
	{
		// Update the bits going in each direction (removing the ones that reach the edge of the board)
		north = BitboardGenerator::north(north) & ~RANK_8;
		south = BitboardGenerator::south(south) & ~RANK_1;
		east = BitboardGenerator::east(east) & ~FILE_H;
		west = BitboardGenerator::west(west) & ~FILE_A;

		occupancyMask |= north | south | east | west;
	}

	return occupancyMask;
}

uint64_t BitboardGenerator::generateBishopOccupancyMask(const uint64_t bishopBitboard)
{
	uint64_t occupancyMask = 0;

	// Directions of bishop movement
	uint64_t northEast = bishopBitboard;
	uint64_t northWest = bishopBitboard;
	uint64_t southEast = bishopBitboard;
	uint64_t southWest = bishopBitboard;

	// Maximum 6 bits in each direction
	for (int i = 0; i < 6; i++)
	{
		// Update the bits going in each direction (removing the ones that reach the edge of the board)
		northEast = BitboardGenerator::northEast(northEast) & (~RANK_8 & ~FILE_H);
		northWest = BitboardGenerator::northWest(northWest) & (~RANK_8 & ~FILE_A);
		southEast = BitboardGenerator::southEast(southEast) & (~RANK_1 & ~FILE_H);
		southWest = BitboardGenerator::southWest(southWest) & (~RANK_1 & ~FILE_A);

		occupancyMask |= northEast | northWest | southEast | southWest;
	}

	return occupancyMask;
}
