#include "ChessEngine.h"
#include <iostream>

ChessEngine::ChessEngine() {
    initializeBitboards();
    initializeSquarePieceTypeArray();
}

uint64_t ChessEngine::getAllPieces() const
{
    uint64_t result = 0;

    for (int color = 0; color < 2; color++)
        for (int type = 0; type < 6; type++)
            result |= pieces[color][type];

    return result;
}

void ChessEngine::initializeSquarePieceTypeArray()
{
    for (int square = 0; square < 64; square++)
    {
        squarePieceType[square] = PieceType::NONE;

        for (int color = 0; color < 2; color++)
            for (int type = 0; type < 6; type++)
                if (pieces[color][type] & (1ULL << square))
                    squarePieceType[square] = static_cast<PieceType>(type);
    }
}

void ChessEngine::initializeBitboards() {
    // Standard chess starting position for each piece type
    pieces[WHITE][PAWN] = 0x000000000000FF00ULL;
    pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    pieces[WHITE][ROOK] = 0x0000000000000081ULL;
    pieces[WHITE][QUEEN] = 0x0000000000000008ULL;
    pieces[WHITE][KING] = 0x0000000000000010ULL;

    pieces[BLACK][PAWN] = 0x00FF000000000000ULL;
    pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
    pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
    pieces[BLACK][ROOK] = 0x8100000000000000ULL;
    pieces[BLACK][QUEEN] = 0x0800000000000000ULL;
    pieces[BLACK][KING] = 0x1000000000000000ULL;

    initializePawnMovesetBitboards();
    initializeKnightMovesetBitboards();
    initializeKingMovesetBitboards();
    
    initializeRookOccupancyMasks();
    initializeRookMovesetBitboards();

    initializeBishopOccupancyMasks();
    initializeBishopMovesetBitboards();
}

void ChessEngine::initializePawnMovesetBitboards()
{
    for (int square = 0; square < 64; square++)
    {
        // Generate pushes
        pawnPushes[WHITE][square] = BitboardGenerator::generatePawnPush(1ULL << square, WHITE);
        pawnPushes[BLACK][square] = BitboardGenerator::generatePawnPush(1ULL << square, BLACK);

        // Generate attacks
        pawnAttacks[WHITE][square] = BitboardGenerator::generatePawnAttack(1ULL << square, WHITE);
        pawnAttacks[BLACK][square] = BitboardGenerator::generatePawnAttack(1ULL << square, BLACK);
    }
}

void ChessEngine::initializeKnightMovesetBitboards()
{
    for (int square = 0; square < 64; square++)
        knightMovement[square] = BitboardGenerator::generateKnightMoveset(1ULL << square);
}

void ChessEngine::initializeKingMovesetBitboards()
{
    for (int square = 0; square < 64; square++)
        kingMovement[square] = BitboardGenerator::generateKingMoveset(1ULL << square);
}

void ChessEngine::initializeRookOccupancyMasks()
{
    for (int square = 0; square < 64; square++)
        rookOccupancyMask[square] = BitboardGenerator::generateRookOccupancyMask(1ULL << square);
}

void ChessEngine::initializeRookMovesetBitboards()
{
    int rookMovementIndex = 0;

    for (int square = 0; square < 64; square++)
    {
        rookSquareOffset[square] = rookMovementIndex;

        uint64_t occupancyMask = BitboardGenerator::generateRookOccupancyMask(1ULL << square);
        uint64_t subset = 0;

        // Carry ripple subset enumeration
        do
        {
            // PEXT operation for hashing occupancy set
            rookMovement[rookSquareOffset[square] + _pext_u64(subset, occupancyMask)] = BitboardGenerator::generateRookMoveset(square, subset);
            rookMovementIndex++;
            
            subset = (subset - occupancyMask) & occupancyMask;
        } while (subset);
    }
}

void ChessEngine::initializeBishopOccupancyMasks()
{
    for (int square = 0; square < 64; square++)
        bishopOccupancyMask[square] = BitboardGenerator::generateBishopOccupancyMask(1ULL << square);
}

void ChessEngine::initializeBishopMovesetBitboards()
{
    int bishopMovementIndex = 0;

    for (int square = 0; square < 64; square++)
    {
        bishopSquareOffset[square] = bishopMovementIndex;

        uint64_t occupancyMask = BitboardGenerator::generateBishopOccupancyMask(1ULL << square);
        uint64_t subset = 0;
        
        do
        {
            // PEXT operation for hashing occupancy set
            bishopMovement[bishopSquareOffset[square] + _pext_u64(subset, occupancyMask)] = BitboardGenerator::generateBishopMoveset(square, subset);
            bishopMovementIndex++;

            subset = (subset - occupancyMask) & occupancyMask;
        } while (subset);
    }
}

void ChessEngine::addPawnMoves(const Color color, MoveList& moveList, const uint64_t ownPieces, const uint64_t enemyPieces) const
{
    uint64_t pawns = pieces[color][PAWN];
    uint64_t allPieces = ownPieces | enemyPieces;
    uint64_t doublePushRank = color == Color::WHITE ? BitboardGenerator::RANK_2 : BitboardGenerator::RANK_7;
    uint64_t promotionRank = color == Color::WHITE ? BitboardGenerator::RANK_7 : BitboardGenerator::RANK_2;

    while (pawns)
    {
        // Get the LSB
        int square = _tzcnt_u64(pawns);

        // Bitboard containing the currently processed pawn
        uint64_t singlePawnBitboard = 1ULL << square;

        // Add pawn pushes
        if (!(pawnPushes[color][square] & allPieces))
        {
            int pushSquare = _tzcnt_u64(pawnPushes[color][square]);

            // Check for promotion possibility
            if (singlePawnBitboard & promotionRank)
            {
                // Add all possible promotions
                moveList.add(Move(square, pushSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::QUEEN));
                moveList.add(Move(square, pushSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::KNIGHT));
                moveList.add(Move(square, pushSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::ROOK));
                moveList.add(Move(square, pushSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::BISHOP));
            }
            else
            {
                // Add a normal pawn push
                moveList.add(Move(square, pushSquare));
            }

            // Check for double push possibility
            if (singlePawnBitboard & doublePushRank)
            {
                if (!(pawnPushes[color][pushSquare] & allPieces))
                    moveList.add(Move(square, _tzcnt_u64(pawnPushes[color][pushSquare])));
            }
        }

        // Add pawn attacks
        uint64_t successfulAttacks = pawnAttacks[color][square] & (enemyPieces & ~pieces[!color][KING]); // The enemy king can not be captured
        while (successfulAttacks)
        {
            int attackedSquare = _tzcnt_u64(successfulAttacks);
            if (singlePawnBitboard & promotionRank)
            {
                // Add all possible promotions
                moveList.add(Move(square, attackedSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::QUEEN));
                moveList.add(Move(square, attackedSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::KNIGHT));
                moveList.add(Move(square, attackedSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::ROOK));
                moveList.add(Move(square, attackedSquare, Move::MoveType::PROMOTION, Move::PromotionPiece::BISHOP));
            }
            else
            {
                // Add a normal pawn attack
                moveList.add(Move(square, attackedSquare));
            }
            successfulAttacks &= successfulAttacks - 1;
        }

        // Remove the LSB
        pawns &= pawns - 1;
    }
}

void ChessEngine::addKnightMoves(const Color color, MoveList& moveList, const uint64_t ownPieces) const
{
    uint64_t knights = pieces[color][KNIGHT];

    while (knights)
    {
        // Get the LSB
        int square = _tzcnt_u64(knights);

        // Add knight moves (the enemy king can not be captured)
        uint64_t possibleMoves = knightMovement[square] & ~ownPieces & ~pieces[!color][KING];
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            moveList.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // Remove the LSB
        knights &= knights - 1;
    }
}

void ChessEngine::addKingMoves(const Color color, MoveList& movelist, const uint64_t ownPieces) const
{
    uint64_t king = pieces[color][KING];

    while (king)
    {
        // Get the LSB
        int square = _tzcnt_u64(king);

        // Add king moves (enemy king can not be captured)
        uint64_t possibleMoves = kingMovement[square] & ~ownPieces & ~pieces[!color][KING];
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // TODO Implement castling

        // Remove the LSB
        king &= king - 1;
    }
}

void ChessEngine::addRookMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const
{
    uint64_t rooks = pieces[color][ROOK];
    uint64_t allPieces = ownPieces | enemyPieces;

    while (rooks)
    {
        // Get the LSB
        int square = _tzcnt_u64(rooks);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPieces & rookOccupancyMask[square], rookOccupancyMask[square])];
        possibleMoves &= ~ownPieces; // Remove the pieces of the same color from the attack set
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // Remove the LSB
        rooks &= rooks - 1;
    }
}

void ChessEngine::addBishopMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const
{
    uint64_t bishops = pieces[color][BISHOP];
    uint64_t allPieces = ownPieces | enemyPieces;

    while (bishops)
    {
        // Get the LSB
        int square = _tzcnt_u64(bishops);

        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPieces & bishopOccupancyMask[square], bishopOccupancyMask[square])];
        possibleMoves &= ~ownPieces; // Remove the pieces of the same color from the attack set
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // Remove the LSB
        bishops &= bishops - 1;
    }
}

void ChessEngine::addQueenMoves(const Color color, MoveList& movelist, const uint64_t ownPieces, const uint64_t enemyPieces) const
{
    uint64_t queens = pieces[color][QUEEN];
    uint64_t allPieces = ownPieces | enemyPieces;

    while (queens)
    {
        // Get the LSB
        int square = _tzcnt_u64(queens);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleRookMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPieces & rookOccupancyMask[square], rookOccupancyMask[square])];
        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleBishopMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPieces & bishopOccupancyMask[square], bishopOccupancyMask[square])];

        // Combine the rook and bishop moves and remove own pieces from attack set
        uint64_t possibleMoves = (possibleRookMoves | possibleBishopMoves) & ~ownPieces;
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // Remove the LSB
        queens &= queens - 1;
    }
}

std::string ChessEngine::bitboardToString(const uint64_t bitboard) const {
    std::string board = "";
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            board += (bitboard & (1ULL << square)) ? "1 " : ". ";
        }
        board += "\n";
    }
    return board;
}

MoveList ChessEngine::getMoves(const Color color) const
{
    MoveList moveList;

    uint64_t ownPieces = pieces[color][PAWN] | pieces[color][KNIGHT] | pieces[color][BISHOP] | pieces[color][ROOK] | pieces[color][QUEEN] | pieces[color][KING];
    uint64_t enemyPieces = pieces[!color][PAWN] | pieces[!color][KNIGHT] | pieces[!color][BISHOP] | pieces[!color][ROOK] | pieces[!color][QUEEN] | pieces[!color][KING];

    addPawnMoves(color, moveList, ownPieces, enemyPieces);
    addKnightMoves(color, moveList, ownPieces);
    addKingMoves(color, moveList, ownPieces);
    addRookMoves(color, moveList, ownPieces, enemyPieces);
    addBishopMoves(color, moveList, ownPieces, enemyPieces);
    addQueenMoves(color, moveList, ownPieces, enemyPieces);

    return moveList;
}

void ChessEngine::makeMove(const Move move, const Color colorToMove)
{
    // Extract move information
    uint8_t toSquare = move.to();
    uint8_t fromSquare = move.from();
    uint64_t toSquareMask = 1ULL << toSquare;
    uint64_t fromSquareMask = 1ULL << fromSquare;
    Move::MoveType moveType = move.moveType();

    // Find moving piece type
    PieceType movingPieceType = squarePieceType[fromSquare];

    if (moveType == Move::MoveType::NORMAL)
    {
        // Move the piece
        pieces[colorToMove][movingPieceType] ^= fromSquareMask;
        pieces[colorToMove][movingPieceType] ^= toSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = squarePieceType[toSquare];

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE && pieces[!colorToMove][capturedPieceType] != PieceType::NONE)
            pieces[!colorToMove][capturedPieceType] ^= toSquareMask;

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = movingPieceType;

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, moveType, capturedPieceType));

        return;
    }

    // TODO Implement special move handling
}

void ChessEngine::undoMove(const Color colorThatMoved)
{
    // Get last move information
    UndoHelper undoHelper = undoStack.top();
    undoStack.pop();

    // Extract move information
    uint8_t toSquare = undoHelper.to();
    uint8_t fromSquare = undoHelper.from();
    uint64_t toSquareMask = 1ULL << toSquare;
    uint64_t fromSquareMask = 1ULL << fromSquare;
    Move::MoveType moveType = undoHelper.moveType();

    // Find moving piece type
    PieceType movingPieceType = squarePieceType[toSquare];

    if (moveType == Move::MoveType::NORMAL)
    {
        // Move the piece
        pieces[colorThatMoved][movingPieceType] ^= toSquareMask;
        pieces[colorThatMoved][movingPieceType] ^= fromSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = static_cast<PieceType>(undoHelper.capturedPieceType());
        if (capturedPieceType != PieceType::NONE)
            pieces[!colorThatMoved][capturedPieceType] ^= toSquareMask;

        squarePieceType[toSquare] = capturedPieceType;
        squarePieceType[fromSquare] = movingPieceType;

        return;
    }

    // TODO Implement special move handling
}

unsigned long long ChessEngine::perft(const int depth, const Color colorToMove)
{
    if (depth == 0)
        return 1;

    MoveList movelist = getMoves(colorToMove);

    if (depth == 1)
        return movelist.numberOfMoves;
    
    unsigned long long result = 0;

    for (int i = 0; i < movelist.numberOfMoves; i++)
    {
        makeMove(movelist.moves[i], colorToMove);
        result += perft(depth - 1, static_cast<Color>(1 -colorToMove));
        undoMove(colorToMove);
    }

    return result;
}
