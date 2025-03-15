#include "BitboardGenerator.h"

uint64_t BitboardGenerator::generateSquaresBetween(const int firstSquare, const int secondSquare)
{
	// Check if the squares are equal
	if (firstSquare == secondSquare)
		return 0ULL;

	// Check if the squares are on the same rank
	if (firstSquare / 8 == secondSquare / 8)
	{
		int leftSquare = std::min(firstSquare, secondSquare);
		int rightSquare = std::max(firstSquare, secondSquare);
		
		uint64_t squaresInBetween = 0ULL;
		for (int i = leftSquare + 1; i <= rightSquare - 1; i++)
			squaresInBetween |= 1ULL << i;

		return squaresInBetween;
	}

	// Check if the squares are on the same file
	if (firstSquare % 8 == secondSquare % 8)
	{
		int belowSquare = std::min(firstSquare, secondSquare);
		int aboveSquare = std::max(firstSquare, secondSquare);

		uint64_t squaresInBetween = 0ULL;
		for (int i = belowSquare + 8; i <= aboveSquare - 8; i += 8)
			squaresInBetween |= 1ULL << i;

		return squaresInBetween;
	}

	// Check if the squares are on the same diagonal
	if ((firstSquare / 8) - (firstSquare % 8) == (secondSquare / 8) - (secondSquare % 8))
	{
		int startSquare = std::min(firstSquare, secondSquare);
		int stopSquare = std::max(firstSquare, secondSquare);

		uint64_t squaresInBetween = 0ULL;
		for (int i = startSquare + 9; i <= stopSquare - 9; i += 9)
			squaresInBetween |= 1ULL << i;

		return squaresInBetween;
	}

	// Check if the squares are on the same anti-diagonal
	if ((firstSquare / 8) + (firstSquare % 8) == (secondSquare / 8) + (secondSquare % 8))
	{
		int startSquare = std::min(firstSquare, secondSquare);
		int stopSquare = std::max(firstSquare, secondSquare);

		uint64_t squaresInBetween = 0ULL;
		for (int i = startSquare + 7; i <= stopSquare - 7; i += 7)
			squaresInBetween |= 1ULL << i;

		return squaresInBetween;
	}

	// Return no squares in between if the given squares are not on the same line or diagonal
	return 0ULL;
}

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

uint64_t BitboardGenerator::generateRookMoveset(const int square, const uint64_t occupancySet)
{
	uint64_t attackSet = 0;

	// Bitboard containing the rook
	const uint64_t rook = 1ULL << square;

	// Go north as much as possible
	uint64_t north = BitboardGenerator::north(rook);
	while (north)
	{
		attackSet |= north;
		
		// Stop at the first set bit in the occupancy set
		if (north & occupancySet)
			break;

		north = BitboardGenerator::north(north);
	}

	// Go south as much as possible
	uint64_t south = BitboardGenerator::south(rook);
	while (south)
	{
		attackSet |= south;

		// Stop at the first set bit in the occupancy set
		if (south & occupancySet)
			break;

		south = BitboardGenerator::south(south);
	}

	// Go east as much as possible
	uint64_t east = BitboardGenerator::east(rook);
	while (east)
	{
		attackSet |= east;

		// Stop at the first set bit in the occupancy set
		if (east & occupancySet)
			break;

		east = BitboardGenerator::east(east);
	}

	// Go west as much as possible
	uint64_t west = BitboardGenerator::west(rook);
	while (west)
	{
		attackSet |= west;

		// Stop at the first set bit in the occupancy set
		if (west & occupancySet)
			break;

		west = BitboardGenerator::west(west);
	}

	return attackSet;
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

uint64_t BitboardGenerator::generateBishopMoveset(const int square, const uint64_t occupancySet)
{
	uint64_t attackSet = 0;

	// Bitboard containing the rook
	const uint64_t bishop = 1ULL << square;

	// Go north-east as much as possible
	uint64_t northEast = BitboardGenerator::north(east(bishop));
	while (northEast)
	{
		attackSet |= northEast;

		// Stop at the first set bit in the occupancy set
		if (northEast & occupancySet)
			break;

		northEast = BitboardGenerator::north(east(northEast));
	}

	// Go north-west as much as possible
	uint64_t northWest = BitboardGenerator::north(west(bishop));
	while (northWest)
	{
		attackSet |= northWest;

		// Stop at the first set bit in the occupancy set
		if (northWest & occupancySet)
			break;

		northWest = BitboardGenerator::north(west(northWest));
	}

	// Go south-east as much as possible
	uint64_t southEast = BitboardGenerator::south(east(bishop));
	while (southEast)
	{
		attackSet |= southEast;

		// Stop at the first set bit in the occupancy set
		if (southEast & occupancySet)
			break;

		southEast = BitboardGenerator::south(east(southEast));
	}

	// Go south-west as much as possible
	uint64_t southWest = BitboardGenerator::south(west(bishop));
	while (southWest)
	{
		attackSet |= southWest;

		// Stop at the first set bit in the occupancy set
		if (southWest & occupancySet)
			break;

		southWest = BitboardGenerator::south(west(southWest));
	}

	return attackSet;
}
