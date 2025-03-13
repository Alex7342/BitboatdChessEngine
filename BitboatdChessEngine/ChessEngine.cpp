#include "ChessEngine.h"
#include <iostream>

ChessEngine::ChessEngine() {
    initializeBitboards();
    initializeSquarePieceTypeArray();
    initializePromotionPieceToPieceTypeArray();
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
        std::cout << square << " " << position[i] << "\n";

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

void ChessEngine::addPawnMoves(const Color color, MoveList& moveList) const
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
                if (!(pawnPushes[color][pushSquare] & allPiecesOnBoard))
                    moveList.add(Move(square, _tzcnt_u64(pawnPushes[color][pushSquare])));
            }
        }

        // Add pawn attacks
        uint64_t successfulAttacks = pawnAttacks[color][square] & (allPieces[color ^ 1] & ~pieces[color ^ 1][KING]); // The enemy king can not be captured
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

void ChessEngine::addKnightMoves(const Color color, MoveList& moveList) const
{
    uint64_t knights = pieces[color][KNIGHT];

    while (knights)
    {
        // Get the LSB
        int square = _tzcnt_u64(knights);

        // Add knight moves (the enemy king can not be captured)
        uint64_t possibleMoves = knightMovement[square] & ~allPieces[color] & ~pieces[color ^ 1][KING];
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

        // TODO Implement castling

        // Remove the LSB
        king &= king - 1;
    }
}

void ChessEngine::addRookMoves(const Color color, MoveList& movelist) const
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

void ChessEngine::addBishopMoves(const Color color, MoveList& movelist) const
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

void ChessEngine::addQueenMoves(const Color color, MoveList& movelist) const
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

bool ChessEngine::isAttacked(const int square, const Color color)
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

    uint64_t allPieces = this->allPieces[WHITE] + this->allPieces[BLACK];

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

MoveList ChessEngine::getMoves(const Color color) const
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

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorToMove ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorToMove ^ 1] ^= toSquareMask;
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = movingPieceType;

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, moveType, capturedPieceType));

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

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorToMove ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorToMove ^ 1] ^= toSquareMask;
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = promotionType;

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, moveType, capturedPieceType));

        return;
    }

    // TODO Implement en passant and castle handling
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

        return;
    }

    // TODO Implement en passant and castle handling
}

unsigned long long ChessEngine::perft(const int depth, const Color colorToMove)
{
    if (depth == 0)
        return 1;

    MoveList movelist = getMoves(colorToMove);

    if (depth == 1)
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
    }
    
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
