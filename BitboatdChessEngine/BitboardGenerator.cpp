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
