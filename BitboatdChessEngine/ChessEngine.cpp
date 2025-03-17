#include "ChessEngine.h"
#include <iostream>

ChessEngine::ChessEngine() {
    initializeBitboards();
    initializeSquarePieceTypeArray();
    initializePromotionPieceToPieceTypeArray();
    initializePositionSpecialStatistics();
}

void ChessEngine::loadFENPosition(const std::string position)
{
    for (int color = 0; color < 2; color++)
    {
        allPieces[color] = 0ULL;
        for (int type = 0; type < 6; type++)
            pieces[color][type] = 0ULL;
    }

    int square = 56;

    for (int i = 0; i < position.size(); i++)
    {
        if ('a' <= position[i] && position[i] <= 'z')
        {
            allPieces[BLACK] ^= 1ULL << square;

            switch (position[i])
            {
            case 'p':
                pieces[BLACK][PAWN] ^= 1ULL << square;
                break;
            case 'n':
                pieces[BLACK][KNIGHT] ^= 1ULL << square;
                break;
            case 'b':
                pieces[BLACK][BISHOP] ^= 1ULL << square;
                break;
            case 'r':
                pieces[BLACK][ROOK] ^= 1ULL << square;
                break;
            case 'q':
                pieces[BLACK][QUEEN] ^= 1ULL << square;
                break;
            case 'k':
                pieces[BLACK][KING] ^= 1ULL << square;
                break;
            default:
                break;
            }

            square++;
        }
        else if ('A' <= position[i] && position[i] <= 'Z')
        {
            allPieces[WHITE] ^= 1ULL << square;

            switch (position[i])
            {
            case 'P':
                pieces[WHITE][PAWN] ^= 1ULL << square;
                break;
            case 'N':
                pieces[WHITE][KNIGHT] ^= 1ULL << square;
                break;
            case 'B':
                pieces[WHITE][BISHOP] ^= 1ULL << square;
                break;
            case 'R':
                pieces[WHITE][ROOK] ^= 1ULL << square;
                break;
            case 'Q':
                pieces[WHITE][QUEEN] ^= 1ULL << square;
                break;
            case 'K':
                pieces[WHITE][KING] ^= 1ULL << square;
                break;
            default:
                break;
            }

            square++;
        }
        else if ('0' <= position[i] && position[i] <= '9')
        {
            int number = 0;

            // Parse the whole number
            while (i < position.size() && '0' <= position[i] && position[i] <= '9')
            {
                number = number * 10 + position[i] - '0';
                i++;
            }

            // Make i the index of the last digit of the number, as the "for" statement will increment it
            i--;

            square += number;
        }
        else if (position[i] == '/')
        {
            // Get to the next rank
            square -= 16;
        }
    }

    initializeSquarePieceTypeArray();
}

uint64_t ChessEngine::getAllPieces() const
{
    return allPieces[WHITE] | allPieces[BLACK];
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

void ChessEngine::initializePromotionPieceToPieceTypeArray()
{
    // Initialize promotion piece to normal piece array
    for (int pieceType = 0; pieceType < 6; pieceType++)
        switch (pieceType)
        {
        case PieceType::QUEEN:
            promotionPieceToPieceType[Move::PromotionPiece::QUEEN] = PieceType::QUEEN;
            break;
        case PieceType::KNIGHT:
            promotionPieceToPieceType[Move::PromotionPiece::KNIGHT] = PieceType::KNIGHT;
            break;
        case PieceType::ROOK:
            promotionPieceToPieceType[Move::PromotionPiece::ROOK] = PieceType::ROOK;
            break;
        case PieceType::BISHOP:
            promotionPieceToPieceType[Move::PromotionPiece::BISHOP] = PieceType::BISHOP;
            break;
        default:
            break;
        }
}

void ChessEngine::initializePositionSpecialStatistics()
{
    // Initialize castling rights
    castlingRights = (1 << 4) - 1;
    whiteCastleQueenSide = 1;
    whiteCastleKingSide = 1 << 1;
    blackCastleQueenSide = 1 << 2;
    blackCastleKingSide = 1 << 3;

    // Initialize en passant target squares
    enPassantTargetBitboard = 0ULL;

    // TODO Initialize 50 move rule, etc
}

void ChessEngine::initializeSquaresBetweenBitboards()
{
    for (int firstSquare = 0; firstSquare < 64; firstSquare++)
        for (int secondSquare = 0; secondSquare < 64; secondSquare++)
            squaresBetween[firstSquare][secondSquare] = BitboardGenerator::generateSquaresBetween(firstSquare, secondSquare);
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

    allPieces[WHITE] = 0x000000000000FFFFULL;
    allPieces[BLACK] = 0xFFFF000000000000ULL;

    initializeSquaresBetweenBitboards();

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

void ChessEngine::addPawnMoves(const Color color, MoveList& moveList, const uint64_t mask) const
{
    uint64_t pawns = pieces[color][PAWN];
    uint64_t allPiecesOnBoard = allPieces[color] | allPieces[color ^ 1];
    uint64_t doublePushRank = color == Color::WHITE ? BitboardGenerator::RANK_2 : BitboardGenerator::RANK_7;
    uint64_t promotionRank = color == Color::WHITE ? BitboardGenerator::RANK_7 : BitboardGenerator::RANK_2;

    while (pawns)
    {
        // Get the LSB
        int square = _tzcnt_u64(pawns);

        // Bitboard containing the currently processed pawn
        uint64_t singlePawnBitboard = 1ULL << square;

        // Add pawn pushes
        if (!(pawnPushes[color][square] & allPiecesOnBoard))
        {
            int pushSquare = _tzcnt_u64(pawnPushes[color][square]);

            if (pawnPushes[color][square] & mask)
            {
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
            }

            // Check for double push possibility
            if (singlePawnBitboard & doublePushRank)
            {
                if (!(pawnPushes[color][pushSquare] & allPiecesOnBoard) && (pawnPushes[color][pushSquare] & mask))
                    moveList.add(Move(square, _tzcnt_u64(pawnPushes[color][pushSquare])));
            }
        }

        // Add pawn attacks
        uint64_t successfulAttacks = (pawnAttacks[color][square] & (allPieces[color ^ 1] & ~pieces[color ^ 1][KING])) & mask; // The enemy king can not be captured
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

        // Add en passant attack
        if (pawnAttacks[color][square] & enPassantTargetBitboard)
        {
            // Add en passant attack
            moveList.add(Move(square, _tzcnt_u64(enPassantTargetBitboard), Move::MoveType::EN_PASSANT));
        }

        // Remove the LSB
        pawns &= pawns - 1;
    }
}

void ChessEngine::addKnightMoves(const Color color, MoveList& moveList, const uint64_t mask) const
{
    uint64_t knights = pieces[color][KNIGHT];

    while (knights)
    {
        // Get the LSB
        int square = _tzcnt_u64(knights);

        // Add knight moves (the enemy king can not be captured)
        uint64_t possibleMoves = knightMovement[square] & ~allPieces[color] & ~pieces[color ^ 1][KING] & mask;
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

void ChessEngine::addKingMoves(const Color color, MoveList& movelist) const
{
    uint64_t king = pieces[color][KING];

    while (king)
    {
        // Get the LSB
        int square = _tzcnt_u64(king);

        // Add king moves (enemy king can not be captured)
        uint64_t possibleMoves = kingMovement[square] & ~allPieces[color] & ~pieces[color ^ 1][KING];
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // TODO Check attacks with an attack bitboard
        if (color == Color::WHITE)
        {
            if (castlingRights & whiteCastleQueenSide)
            {
                if (!(squaresBetween[0][4] & (allPieces[color] | allPieces[color ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[1][4] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(color ^ 1)))
                        {
                            canCastle = false;
                            break;
                        }
                        castleBitboard &= castleBitboard - 1;
                    }

                    if (canCastle)
                        movelist.add(Move(square, 2, Move::MoveType::CASTLE));
                }
            }

            if (castlingRights & whiteCastleKingSide)
            {
                if (!(squaresBetween[7][4] & (allPieces[color] | allPieces[color ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[7][4] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(color ^ 1)))
                        {
                            canCastle = false;
                            break;
                        }
                        castleBitboard &= castleBitboard - 1;
                    }

                    if (canCastle)
                        movelist.add(Move(square, 6, Move::MoveType::CASTLE));
                }
            }
        }
        else if (color == Color::BLACK)
        {
            if (castlingRights & blackCastleQueenSide)
            {
                if (!(squaresBetween[56][60] & (allPieces[color] | allPieces[color ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[57][60] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(color ^ 1)))
                        {
                            canCastle = false;
                            break;
                        }
                        castleBitboard &= castleBitboard - 1;
                    }

                    if (canCastle)
                        movelist.add(Move(square, 58, Move::MoveType::CASTLE));
                }
            }

            if (castlingRights & blackCastleKingSide)
            {
                if (!(squaresBetween[63][60] & (allPieces[color] | allPieces[color ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[63][60] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(color ^ 1)))
                        {
                            canCastle = false;
                            break;
                        }
                        castleBitboard &= castleBitboard - 1;
                    }

                    if (canCastle)
                        movelist.add(Move(square, 62, Move::MoveType::CASTLE));
                }
            }
        }

        // Remove the LSB
        king &= king - 1;
    }
}

void ChessEngine::addRookMoves(const Color color, MoveList& movelist, const uint64_t mask) const
{
    uint64_t rooks = pieces[color][ROOK];
    uint64_t allPiecesOnBoard = allPieces[color] | allPieces[color ^ 1];

    while (rooks)
    {
        // Get the LSB
        int square = _tzcnt_u64(rooks);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[square], rookOccupancyMask[square])];
        possibleMoves &= ~allPieces[color]; // Remove the pieces of the same color from the attack set
        possibleMoves &= mask; // Only select moves within the mask
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

void ChessEngine::addBishopMoves(const Color color, MoveList& movelist, const uint64_t mask) const
{
    uint64_t bishops = pieces[color][BISHOP];
    uint64_t allPiecesOnBoard = allPieces[color] | allPieces[color ^ 1];

    while (bishops)
    {
        // Get the LSB
        int square = _tzcnt_u64(bishops);

        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[square], bishopOccupancyMask[square])];
        possibleMoves &= ~allPieces[color]; // Remove the pieces of the same color from the attack set
        possibleMoves &= mask; // Only select moves within the mask
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

void ChessEngine::addQueenMoves(const Color color, MoveList& movelist, const uint64_t mask) const
{
    uint64_t queens = pieces[color][QUEEN];
    uint64_t allPiecesOnBoard = allPieces[color] | allPieces[color ^ 1];

    while (queens)
    {
        // Get the LSB
        int square = _tzcnt_u64(queens);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleRookMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[square], rookOccupancyMask[square])];
        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleBishopMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[square], bishopOccupancyMask[square])];

        // Combine the rook and bishop moves and remove own pieces from attack set
        uint64_t possibleMoves = (possibleRookMoves | possibleBishopMoves) & ~allPieces[color];
        possibleMoves &= mask; // Only select moves within the mask
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

bool ChessEngine::isAttacked(const int square, const Color color) const
{
    // Check for pawn attacks
    if (pawnAttacks[color ^ 1][square] & pieces[color][PAWN])
        return true;

    // Check for knight attacks
    if (knightMovement[square] & pieces[color][KNIGHT])
        return true;

    // Check for king attacks
    if (kingMovement[square] & pieces[color][KING])
        return true;

    uint64_t allPieces = this->allPieces[WHITE] | this->allPieces[BLACK];

    // Check for rook attacks (queens included)
    uint64_t rookAttacks = rookMovement[rookSquareOffset[square] + _pext_u64(allPieces & rookOccupancyMask[square], rookOccupancyMask[square])];
    if (rookAttacks & (pieces[color][ROOK] | pieces[color][QUEEN]))
        return true;

    // Check for bishop attacks (queens included)
    uint64_t bishopAttacks = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPieces & bishopOccupancyMask[square], bishopOccupancyMask[square])];
    if (bishopAttacks & (pieces[color][BISHOP] | pieces[color][QUEEN]))
        return true;

    return false;
}

uint64_t ChessEngine::getAttacksBitboard(const int square, const Color color) const
{
    // Get the other color
    const Color otherColor = static_cast<Color>(color ^ 1);

    // Bitboard of the attacking squares
    uint64_t attackingSquares = 0ULL;

    // Count pawn attacks
    attackingSquares |= (pawnAttacks[otherColor][square] & pieces[color][PAWN]);
    if ((attackingSquares & (attackingSquares - 1))) // Check if there are 2 attacks already (this is the maximum number of attacks)
        return attackingSquares;

    // Count knight attacks
    attackingSquares |= (knightMovement[square] & pieces[color][KNIGHT]);
    if ((attackingSquares & (attackingSquares - 1))) // Check if there are 2 attacks already (this is the maximum number of attacks)
        return attackingSquares;

    uint64_t allPieces = this->allPieces[WHITE] | this->allPieces[BLACK];

    // Count rook attacks (queens included)
    uint64_t rookAttacks = rookMovement[rookSquareOffset[square] + _pext_u64(allPieces & rookOccupancyMask[square], rookOccupancyMask[square])];
    attackingSquares |= (rookAttacks & (pieces[color][ROOK] | pieces[color][QUEEN]));
    if ((attackingSquares & (attackingSquares - 1))) // Check if there are 2 attacks already (this is the maximum number of attacks)
        return attackingSquares;

    // Count attacks (queens included)
    uint64_t bishopAttacks = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPieces & bishopOccupancyMask[square], bishopOccupancyMask[square])];
    attackingSquares |= (bishopAttacks & (pieces[color][BISHOP] | pieces[color][QUEEN]));
    if ((attackingSquares & (attackingSquares - 1))) // Check if there are 2 attacks already (this is the maximum number of attacks)
        return attackingSquares;

    return attackingSquares;
}

MoveList ChessEngine::getPseudolegalMovesInCheck(const Color color, const uint64_t attackingSquares) const
{
    MoveList movelist;
    int kingSquare = _tzcnt_u64(pieces[color][KING]);

    if ((attackingSquares & (attackingSquares - 1)) == 0) // Only one piece attacking the king
    {
        // Generate moves that block the attack or capture the attacker
        int attackingSquare = _tzcnt_u64(attackingSquares);
        uint64_t mask = squaresBetween[kingSquare][attackingSquare] | attackingSquares;

        addKingMoves(color, movelist);
        addPawnMoves(color, movelist, mask);
        addKnightMoves(color, movelist, mask);
        addRookMoves(color, movelist, mask);
        addBishopMoves(color, movelist, mask);
        addQueenMoves(color, movelist, mask);

        return movelist;
    }
    else // Two pieces attacking the king
    {
        // Generate only king moves (no castle)
        uint64_t possibleMoves = kingMovement[kingSquare] & ~allPieces[color] & ~pieces[color ^ 1][KING];
        while (possibleMoves)
        {
            int escapeSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(kingSquare, escapeSquare));
            possibleMoves &= possibleMoves - 1;
        }
    }

    return movelist;
}

bool ChessEngine::compareMoves(const Move firstMove, const Move secondMove) const
{
    if (pieceValue[squarePieceType[firstMove.to()]] > pieceValue[squarePieceType[secondMove.to()]])
        return true;

    if (pieceValue[squarePieceType[firstMove.to()]] == pieceValue[squarePieceType[secondMove.to()]])
        if (pieceValue[squarePieceType[firstMove.from()]] < pieceValue[squarePieceType[secondMove.from()]])
            return true;

    return false;
}

void ChessEngine::sortMoves(MoveList& movelist) const
{
    std::sort(movelist.moves, movelist.moves + movelist.numberOfMoves, [&](const Move& a, const Move& b) { return this->compareMoves(a, b); });
}

int ChessEngine::evaluate() const
{
    int result = 0;

    // Add white's score to the result
    for (int type = 0; type < 6; type++)
    {
        uint64_t bitboard = pieces[WHITE][type];

        while (bitboard)
        {
            int square = _tzcnt_u64(bitboard);

            result += pieceValue[type] + positionValue[type][square];

            bitboard &= bitboard - 1;
        }
    }

    // Subtract black's score from the result
    for (int type = 0; type < 6; type++)
    {
        uint64_t bitboard = pieces[BLACK][type];

        while (bitboard)
        {
            int square = 63 - _tzcnt_u64(bitboard);

            result -= pieceValue[type] + positionValue[type][square];

            bitboard &= bitboard - 1;
        }
    }

    return result;
}

ChessEngine::SearchResult ChessEngine::negamax(int alpha, int beta, const int depth, const Color colorToMove)
{
    if (depth == 0)
    {
        int color = colorToMove == Color::WHITE ? 1 : -1;
        return SearchResult(evaluate() * color);
    }

    SearchResult result(INT_MIN);

    MoveList moves = getPseudolegalMoves(colorToMove);
    for (int i = 0; i < moves.numberOfMoves; i++)
    {
        makeMove(moves.moves[i], colorToMove);

        // Check if the move is legal
        if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
        {
            SearchResult moveResult(moves.moves[i], -negamax(-beta, -alpha, depth - 1, static_cast<Color>(colorToMove ^ 1)).score);
            
            if (moveResult.score > result.score)
            {
                result = moveResult;

                if (moveResult.score > alpha)
                    alpha = moveResult.score;
            }

            // TODO Fix alpha beta pruning
            if (moveResult.score >= beta)
            {
                undoMove(colorToMove);
                return result;
            }
        }

        undoMove(colorToMove);
    }

    return result;
}

ChessEngine::SearchResult ChessEngine::minimax(int alpha, int beta, const int depth, const Color colorToMove)
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - this->searchStartTime).count();
    if (duration >= this->timeLimitInMilliseconds)
    {
        this->stopSearch = true;
        return SearchResult();
    }

    if (depth == 0)
        return SearchResult(evaluate());

    uint64_t squaresAttackingKing = getAttacksBitboard(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1));
    MoveList moves = squaresAttackingKing ? getPseudolegalMovesInCheck(colorToMove, squaresAttackingKing) : getPseudolegalMoves(colorToMove);
    sortMoves(moves);
    
    if (colorToMove == Color::WHITE)
    {
        SearchResult result(INT_MIN);

        for (int i = 0; i < moves.numberOfMoves; i++)
        {
            makeMove(moves.moves[i], colorToMove);

            // Check if the move is legal
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            {
                SearchResult moveResult(moves.moves[i], minimax(alpha, beta, depth - 1, Color::BLACK).score);

                if (moveResult.score > result.score)
                {
                    result = moveResult;

                    if (moveResult.score > alpha)
                        alpha = moveResult.score;
                }
                
                if (moveResult.score >= beta)
                {
                    undoMove(colorToMove);
                    return result;
                }
            }

            undoMove(colorToMove);
        }

        if (result.score == INT_MIN) // No legal move found
        {
            if (squaresAttackingKing) // If the king is in check then it is checkmate
                result.score = CHECKMATE_SCORE[colorToMove];
            else // If the king is not in check them it is stalemate
                result.score = 0;
        }

        return result;
    }
    else
    {
        SearchResult result(INT_MAX);

        for (int i = 0; i < moves.numberOfMoves; i++)
        {
            makeMove(moves.moves[i], colorToMove);

            // Check if the move is legal
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            {
                SearchResult moveResult(moves.moves[i], minimax(alpha, beta, depth - 1, Color::WHITE).score);

                if (moveResult.score < result.score)
                {
                    result = moveResult;

                    if (moveResult.score < beta)
                        beta = moveResult.score;
                }

                if (moveResult.score <= alpha)
                {
                    undoMove(colorToMove);
                    return result;
                }
            }

            undoMove(colorToMove);
        }

        if (result.score == INT_MAX) // No legal move found
        {
            if (squaresAttackingKing) // If the king is in check then it is checkmate
                result.score = CHECKMATE_SCORE[colorToMove];
            else // If the king is not in check them it is stalemate
                result.score = 0;
        }

        return result;
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

std::string ChessEngine::getSquareNotation(const int square) const
{
    if (square < 0 || square > 63)
        return "Invalid square";

    char file = 'a' + (square % 8); // Get file (column) from 'a' to 'h'
    int rank = 1 + (square / 8);    // Get rank (row) from 1 to 8

    return std::string(1, file) + std::to_string(rank);
}

MoveList ChessEngine::getPseudolegalMoves(const Color color) const
{
    MoveList moveList;

    addPawnMoves(color, moveList);
    addKnightMoves(color, moveList);
    addKingMoves(color, moveList);
    addRookMoves(color, moveList);
    addBishopMoves(color, moveList);
    addQueenMoves(color, moveList);

    return moveList;
}

MoveList ChessEngine::getLegalMoves(const Color color)
{
    MoveList pseudolegalMoves = getPseudolegalMoves(color);
    MoveList legalMoves;

    for (int i = 0; i < pseudolegalMoves.numberOfMoves; i++)
    {
        makeMove(pseudolegalMoves.moves[i], color);
        if (!isAttacked(_tzcnt_u64(pieces[color][KING]), static_cast<Color>(color ^ 1)))
            legalMoves.add(pseudolegalMoves.moves[i]);
        undoMove(color);
    }

    return legalMoves;
}

void ChessEngine::makeMove(const Move move, const Color colorToMove)
{
    // Extract move information
    uint8_t toSquare = move.to();
    uint8_t fromSquare = move.from();
    uint64_t toSquareMask = 1ULL << toSquare;
    uint64_t fromSquareMask = 1ULL << fromSquare;
    Move::MoveType moveType = move.moveType();

    if (moveType == Move::MoveType::NORMAL)
    {
        // Find moving piece type
        PieceType movingPieceType = squarePieceType[fromSquare];

        // Move the piece
        pieces[colorToMove][movingPieceType] ^= fromSquareMask;
        pieces[colorToMove][movingPieceType] ^= toSquareMask;
        allPieces[colorToMove] ^= fromSquareMask;
        allPieces[colorToMove] ^= toSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = squarePieceType[toSquare];
        
        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, moveType, capturedPieceType));

        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorToMove ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorToMove ^ 1] ^= toSquareMask;

            if (capturedPieceType == PieceType::ROOK)
            {
                switch (toSquare)
                {
                case 0:
                    castlingRights &= ~whiteCastleQueenSide;
                    break;
                case 7:
                    castlingRights &= ~whiteCastleKingSide;
                    break;
                case 56:
                    castlingRights &= ~blackCastleQueenSide;
                    break;
                case 63:
                    castlingRights &= ~blackCastleKingSide;
                }
            }
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = movingPieceType;

        // Update castling rights
        if (movingPieceType == KING)
        {
            if (colorToMove == Color::WHITE)
                castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
            else if (colorToMove == Color::BLACK)
                castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
        }
        else if (movingPieceType == ROOK)
        {
            switch (fromSquare)
            {
            case 0:
                castlingRights &= ~whiteCastleQueenSide;
                break;
            case 7:
                castlingRights &= ~whiteCastleKingSide;
                break;
            case 56:
                castlingRights &= ~blackCastleQueenSide;
                break;
            case 63:
                castlingRights &= ~blackCastleKingSide;
            }
        }
        // Update en passant square if a pawn makes a double push
        else if (movingPieceType == PAWN && squaresBetween[fromSquare][toSquare])
        {
            enPassantTargetBitboard = squaresBetween[fromSquare][toSquare];
        }

        return;
    }

    if (moveType == Move::MoveType::PROMOTION)
    {
        // Get promotion type
        PieceType promotionType = promotionPieceToPieceType[move.promotionPiece()];

        // Move and promote the pawn
        pieces[colorToMove][PAWN] ^= fromSquareMask;
        pieces[colorToMove][promotionType] ^= toSquareMask;
        allPieces[colorToMove] ^= fromSquareMask;
        allPieces[colorToMove] ^= toSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = squarePieceType[toSquare];

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, moveType, capturedPieceType));

        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorToMove ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorToMove ^ 1] ^= toSquareMask;

            if (capturedPieceType == PieceType::ROOK)
            {
                switch (toSquare)
                {
                case 0:
                    castlingRights &= ~whiteCastleQueenSide;
                    break;
                case 7:
                    castlingRights &= ~whiteCastleKingSide;
                    break;
                case 56:
                    castlingRights &= ~blackCastleQueenSide;
                    break;
                case 63:
                    castlingRights &= ~blackCastleKingSide;
                }
            }
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = promotionType;

        return;
    }

    if (moveType == Move::MoveType::CASTLE)
    {
        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, moveType, PieceType::NONE));

        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        uint8_t rookToSquare = 0, rookFromSquare = 0;
        uint64_t rookToSquareMask = 0, rookFromSquareMask = 0;

        // Identify the castle move
        switch (toSquare)
        {
        case 2: // White queen side
            rookFromSquare = 0, rookToSquare = 3;
            rookFromSquareMask = 1ULL, rookToSquareMask = 1ULL << 3;
            castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
            break;

        case 6: // White king side
            rookFromSquare = 7, rookToSquare = 5;
            rookFromSquareMask = 1ULL << 7, rookToSquareMask = 1ULL << 5;
            castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
            break;

        case 58: // Black queen side
            rookFromSquare = 56, rookToSquare = 59;
            rookFromSquareMask = 1ULL << 56, rookToSquareMask = 1ULL << 59;
            castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
            break;

        case 62: // Black king side
            rookFromSquare = 63, rookToSquare = 61;
            rookFromSquareMask = 1ULL << 63, rookToSquareMask = 1ULL << 61;
            castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
            break;
        }

        // Move the king
        pieces[colorToMove][KING] ^= fromSquareMask;
        pieces[colorToMove][KING] ^= toSquareMask;
        allPieces[colorToMove] ^= fromSquareMask;
        allPieces[colorToMove] ^= toSquareMask;

        // Move the rook
        pieces[colorToMove][ROOK] ^= rookFromSquareMask;
        pieces[colorToMove][ROOK] ^= rookToSquareMask;
        allPieces[colorToMove] ^= rookFromSquareMask;
        allPieces[colorToMove] ^= rookToSquareMask;

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = PieceType::KING;
        squarePieceType[rookFromSquare] = PieceType::NONE;
        squarePieceType[rookToSquare] = PieceType::ROOK;

        return;
    }

    if (moveType == Move::MoveType::EN_PASSANT)
    {
        // Move the pawn
        pieces[colorToMove][PAWN] ^= fromSquareMask;
        pieces[colorToMove][PAWN] ^= toSquareMask;
        allPieces[colorToMove] ^= fromSquareMask;
        allPieces[colorToMove] ^= toSquareMask;

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, moveType, PieceType::PAWN));

        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Compute the captured pawn coordinates
        int capturedPawnSquare = 0;
        uint64_t capturedPawnMask = 0ULL;
        if (colorToMove == Color::WHITE)
        {
            // Go one square down
            capturedPawnSquare = toSquare - 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }
        else if (colorToMove == Color::BLACK)
        {
            // Go one square up
            capturedPawnSquare = toSquare + 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }

        // Capture the enemy pawn
        pieces[colorToMove ^ 1][PAWN] ^= capturedPawnMask;
        allPieces[colorToMove ^ 1] ^= capturedPawnMask;

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = PieceType::PAWN;
        squarePieceType[capturedPawnSquare] = PieceType::NONE;

        return;
    }
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

    if (moveType == Move::MoveType::NORMAL)
    {
        // Find moving piece type
        PieceType movingPieceType = squarePieceType[toSquare];

        // Move the piece back
        pieces[colorThatMoved][movingPieceType] ^= toSquareMask;
        pieces[colorThatMoved][movingPieceType] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = static_cast<PieceType>(undoHelper.capturedPieceType());

        // Revert the capture of the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorThatMoved ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorThatMoved ^ 1] ^= toSquareMask;
        }

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = capturedPieceType;
        squarePieceType[fromSquare] = movingPieceType;

        // Restore the previous castling rights and en passant target bitboard
        castlingRights = undoHelper.castlingRights();
        enPassantTargetBitboard = undoHelper.enPassantBitboard();

        return;
    }

    if (moveType == Move::MoveType::PROMOTION)
    {
        // Find moving piece type
        PieceType movingPieceType = squarePieceType[toSquare];

        // Move the piece back
        pieces[colorThatMoved][movingPieceType] ^= toSquareMask;
        pieces[colorThatMoved][PAWN] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;

        // Find captured piece type
        PieceType capturedPieceType = static_cast<PieceType>(undoHelper.capturedPieceType());

        // Revert the capture of the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorThatMoved ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorThatMoved ^ 1] ^= toSquareMask;
        }

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = capturedPieceType;
        squarePieceType[fromSquare] = PieceType::PAWN;

        // Restore the previous castling rights and en passant target bitboard
        castlingRights = undoHelper.castlingRights();
        enPassantTargetBitboard = undoHelper.enPassantBitboard();

        return;
    }

    if (moveType == Move::MoveType::CASTLE)
    {
        uint8_t rookToSquare = 0, rookFromSquare = 0;
        uint64_t rookToSquareMask = 0, rookFromSquareMask = 0;

        // Identify the castle move
        switch (toSquare)
        {
        case 2: // White queen side
            rookFromSquare = 0, rookToSquare = 3;
            rookFromSquareMask = 1ULL, rookToSquareMask = 1ULL << 3;
            break;

        case 6: // White king side
            rookFromSquare = 7, rookToSquare = 5;
            rookFromSquareMask = 1ULL << 7, rookToSquareMask = 1ULL << 5;
            break;

        case 58: // Black queen side
            rookFromSquare = 56, rookToSquare = 59;
            rookFromSquareMask = 1ULL << 56, rookToSquareMask = 1ULL << 59;
            break;

        case 62: // Black king side
            rookFromSquare = 63, rookToSquare = 61;
            rookFromSquareMask = 1ULL << 63, rookToSquareMask = 1ULL << 61;
            break;
        }

        // Move the king back
        pieces[colorThatMoved][KING] ^= toSquareMask;
        pieces[colorThatMoved][KING] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;

        // Move the rook back
        pieces[colorThatMoved][ROOK] ^= rookToSquareMask;
        pieces[colorThatMoved][ROOK] ^= rookFromSquareMask;
        allPieces[colorThatMoved] ^= rookToSquareMask;
        allPieces[colorThatMoved] ^= rookFromSquareMask;

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = PieceType::NONE;
        squarePieceType[fromSquare] = PieceType::KING;
        squarePieceType[rookToSquare] = PieceType::NONE;
        squarePieceType[rookFromSquare] = PieceType::ROOK;

        // Restore the previous castling rights and en passant target bitboard
        castlingRights = undoHelper.castlingRights();
        enPassantTargetBitboard = undoHelper.enPassantBitboard();

        return;
    }

    if (moveType == Move::MoveType::EN_PASSANT)
    {
        // Move the pawn back
        pieces[colorThatMoved][PAWN] ^= toSquareMask;
        pieces[colorThatMoved][PAWN] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;

        // Compute the captured pawn coordinates
        int capturedPawnSquare = 0;
        uint64_t capturedPawnMask = 0ULL;
        if (colorThatMoved == Color::WHITE)
        {
            // Go one square down
            capturedPawnSquare = toSquare - 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }
        else if (colorThatMoved == Color::BLACK)
        {
            // Go one square up
            capturedPawnSquare = toSquare + 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }

        // Revert the capture of the enemy pawn
        pieces[colorThatMoved ^ 1][PAWN] ^= capturedPawnMask;
        allPieces[colorThatMoved ^ 1] ^= capturedPawnMask;

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::PAWN;
        squarePieceType[toSquare] = PieceType::NONE;
        squarePieceType[capturedPawnSquare] = PieceType::PAWN;

        // Restore the previous castling rights and en passant target bitboard
        castlingRights = undoHelper.castlingRights();
        enPassantTargetBitboard = undoHelper.enPassantBitboard();

        return;
    }
}

unsigned long long ChessEngine::perft(const int depth, const Color colorToMove)
{
    if (depth == 0)
        return 1;

    uint64_t squaresAttackingKing = getAttacksBitboard(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1));
    MoveList movelist = squaresAttackingKing ? getPseudolegalMovesInCheck(colorToMove, squaresAttackingKing) : getPseudolegalMoves(colorToMove);

    /*if (depth == 1)
    {
        unsigned long long result = 0;
        for (int i = 0; i < movelist.numberOfMoves; i++)
        {
            makeMove(movelist.moves[i], colorToMove);
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
                result++;
            undoMove(colorToMove);
        }
        return result;
    }*/
    
    unsigned long long result = 0;

    for (int i = 0; i < movelist.numberOfMoves; i++)
    {
        makeMove(movelist.moves[i], colorToMove);
        if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            result += perft(depth - 1, static_cast<Color>(1 ^ colorToMove));
        undoMove(colorToMove);
    }

    return result;
}

ChessEngine::SearchResult ChessEngine::search(const int depth, const Color colorToMove)
{
    //return negamax(INT_MIN, INT_MAX, depth, colorToMove);
    return minimax(INT_MIN, INT_MAX, depth, colorToMove);
}

ChessEngine::SearchResult ChessEngine::iterativeDeepeningSearch(const Color colorToMove, const int timeLimit)
{
    this->searchStartTime = std::chrono::high_resolution_clock::now();
    this->timeLimitInMilliseconds = timeLimit;
    this->stopSearch = false;
    SearchResult bestMove;

    for (int depth = 1; /*TODO Choose depth limit */!this->stopSearch; depth++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        SearchResult result = this->minimax(INT_MIN, INT_MAX, depth, colorToMove);
        auto stop = std::chrono::high_resolution_clock::now();

        if (!this->stopSearch)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            std::cout << "Depth " << depth << " reached in " << duration << "ms.\n";
        }

        if (!this->stopSearch)
            bestMove = result;
    }

    return bestMove;
}
