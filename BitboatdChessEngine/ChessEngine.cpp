#include "ChessEngine.h"
#include <iostream>

ChessEngine::ChessEngine() {
    initializeBitboards();
    initializeSquarePieceTypeArray();
    initializePromotionPieceToPieceTypeArray();
    initializePositionSpecialStatistics();
    initializeZobristHash();
    initializeMoveOrderingTables();
    initializeTimeLimits();

    this->transpositionTable = new TranspositionTableEntry[this->transpositionTableSize];
    this->activePlayer = Color::WHITE;

    this->previousPositionsSize = 0;
    this->previousPositions = new uint64_t[18000];
}

ChessEngine::~ChessEngine()
{
    delete[] transpositionTable;
    delete[] rookMovement;
    delete[] bishopMovement;
    delete[] previousPositions;
}

void ChessEngine::loadFENPosition(const std::string position)
{
    for (int color = 0; color < 2; color++)
    {
        allPieces[color] = 0ULL;
        for (int type = 0; type < 6; type++)
            pieces[color][type] = 0ULL;
    }

    this->previousPositionsSize = 0;

    int square = 56;

    int i = 0;
    for (i = 0; i < position.size() && position[i] != ' '; i++)
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

    // Skip all spaces
    while (i < position.size() && position[i] == ' ')
        i++;

    // Get the active player
    if (i < position.size())
    {
        if (position[i] == 'w')
            this->activePlayer = Color::WHITE;
        else if (position[i] == 'b')
            this->activePlayer = Color::BLACK;
        
        i++;
    }

    // Skip all spaces
    while (i < position.size() && position[i] == ' ')
        i++;

    // Get castling rights
    if (i < position.size())
    {
        if (position[i] == '-')
        {
            this->castlingRights = 0;
            i++;
        }
        else
        {
            this->castlingRights = 0;
            while (i < position.size() && position[i] != ' ')
            {
                switch (position[i])
                {
                case 'K':
                    this->castlingRights |= this->whiteCastleKingSide;
                    break;
                case 'Q':
                    this->castlingRights |= this->whiteCastleQueenSide;
                    break;
                case 'k':
                    this->castlingRights |= this->blackCastleKingSide;
                    break;
                case 'q':
                    this->castlingRights |= this->blackCastleQueenSide;
                    break;
                }
                i++;
            }
        }
    }

    // Skip all spaces
    while (i < position.size() && position[i] == ' ')
        i++;

    // Get the en passant target square
    if (i < position.size())
    {
        if (position[i] == '-')
        {
            this->enPassantTargetBitboard = 0ULL;
            i++;
        }
        else
        {
            // Get the file
            int file = position[i] - 'a';

            // Get the rank
            int rank = 0;
            i++;
            if (i < position.size())
                rank = position[i] - '1';

            int square = 8 * rank + file; // Get the square;
            this->enPassantTargetBitboard = 1ULL << square; // Update the en passant target square

            i++;
        }
    }

    // Skip all spaces
    while (i < position.size() && position[i] == ' ')
        i++;

    // Get the halfmove clock
    int halfmoves = 0;
    while (i < position.size() && '0' <= position[i] && position[i] <= '9')
    {
        halfmoves = halfmoves * 10 + position[i] - '0';
        i++;
    }
    this->halfmoveClock = halfmoves;

    // Skip all spaces
    while (i < position.size() && position[i] == ' ')
        i++;

    // Get the halfmove clock
    int fullmoves = 0;
    while (i < position.size() && '0' <= position[i] && position[i] <= '9')
    {
        fullmoves = fullmoves * 10 + position[i] - '0';
        i++;
    }
    this->fullmoveCounter = fullmoves;

    initializeSquarePieceTypeArray();

    // Change the zobrist hash
    this->boardZobristHash = 0ULL;

    // Add pieces
    for (int square = 0; square < 64; square++)
        this->boardZobristHash ^= this->pieceZobristHash[squarePieceType[square]][square];

    // Add castling rights
    this->boardZobristHash ^= this->castlingRightsZobristHash[this->castlingRights];

    // Add en passant square
    if (this->enPassantTargetBitboard)
        this->boardZobristHash ^= this->enPassantTargetSquareZobristHash[_tzcnt_u64(this->enPassantTargetBitboard)];
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

    // Initialize halfmove and full move counters
    halfmoveClock = 0;
    fullmoveCounter = 0;
}

void ChessEngine::initializeZobristHash()
{
    std::random_device randomDevice;
    std::mt19937_64 generator(randomDevice());
    std::uniform_int_distribution<uint64_t> distribution(0, UINT64_MAX);

    // Piece zobrist hashes
    for (int type = 0; type < 6; type++)
        for (int square = 0; square < 64; square++)
            this->pieceZobristHash[type][square] = distribution(generator);

    // Castling rights zobrist hashes
    for (int i = 0; i < 16; i++)
        this->castlingRightsZobristHash[i] = distribution(generator);

    // En passant zobrist hashes
    for (int square = 0; square < 64; square++)
        this->enPassantTargetSquareZobristHash[square] = distribution(generator);


    // Total board zobrist hash
    this->boardZobristHash = 0ULL;
    
    // Add pieces
    for (int square = 0; square < 64; square++)
        this->boardZobristHash ^= this->pieceZobristHash[squarePieceType[square]][square];

    // Add castling rights
    this->boardZobristHash ^= this->castlingRightsZobristHash[this->castlingRights];

    // Add en passant square
    if (this->enPassantTargetBitboard)
        this->boardZobristHash ^= this->enPassantTargetSquareZobristHash[_tzcnt_u64(this->enPassantTargetBitboard)];
}

void ChessEngine::initializeMoveOrderingTables()
{
    this->maxHistoryValueReached = false;
    for (int color = 0; color < 2; color++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                this->historyTable[color][from][to] = 0;
}

void ChessEngine::initializeSquaresBetweenBitboards()
{
    for (int firstSquare = 0; firstSquare < 64; firstSquare++)
        for (int secondSquare = 0; secondSquare < 64; secondSquare++)
            squaresBetween[firstSquare][secondSquare] = BitboardGenerator::generateSquaresBetween(firstSquare, secondSquare);
}

void ChessEngine::decayHistoryTable()
{
    for (int color = 0; color < 2; color++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                this->historyTable[color][from][to] /= 2;
}

void ChessEngine::updateHistoryTable(const Color color, const Move move, const int depth)
{
    int from = move.from();
    int to = move.to();
    historyTable[color][from][to] = std::min(historyTable[color][from][to] + depth * depth, MAX_HISTORY_VALUE);
    if (historyTable[color][from][to] == MAX_HISTORY_VALUE)
        maxHistoryValueReached = true;
}

void ChessEngine::updateKillerMoves(const Move move, const int ply)
{
    if (move == this->killerMoves[ply][0] || move == this->killerMoves[ply][1])
        return;

    if (move != this->killerMoves[ply][0]) // Replace the first move in the table
    {
        this->killerMoves[ply][1] = this->killerMoves[ply][0];
        this->killerMoves[ply][0] = move;
    }
    else // Replace the second move in the table
    {
        this->killerMoves[ply][1] = move;
    }
}

void ChessEngine::clearKillerMoves()
{
    memset(this->killerMoves, 0, sizeof(killerMoves));
}

uint64_t ChessEngine::getXrayAttacksToSquare(const int square, const Color color) const
{
    uint64_t allAttacks = 0ULL;

    // Check for pawn attacks
    allAttacks |= pieces[color][PAWN] & pawnAttacks[color ^ 1][square];

    // Check for knight attacks
    allAttacks |= pieces[color][KNIGHT] & knightMovement[square];
    
    uint64_t allPiecesOnBoard = allPieces[WHITE] | allPieces[BLACK];

    // Check for bishop attacks
    uint64_t possibleBishopMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPiecesOnBoard & ~pieces[color][BISHOP] & ~pieces[color][QUEEN] & bishopOccupancyMask[square], bishopOccupancyMask[square])];
    allAttacks |= pieces[color][BISHOP] & possibleBishopMoves;

    // Check for rook attacks
    uint64_t possibleRookMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPiecesOnBoard & ~pieces[color][ROOK] & ~pieces[color][QUEEN] & rookOccupancyMask[square], rookOccupancyMask[square])];
    allAttacks |= pieces[color][ROOK] & possibleRookMoves;
    

    // Check for queen attacks
    allAttacks |= pieces[color][QUEEN] & (possibleRookMoves | possibleBishopMoves);

    // Check for king attacks
    allAttacks |= pieces[color][KING] & kingMovement[square];

    return allAttacks;
}

int ChessEngine::SEE(const int square, const Color color) const
{
    int gains[MAX_SEE_DEPTH] = { 0 };
    int depth = 0;

    int value = pieceValue[squarePieceType[square]];

    uint64_t attackers[2];
    int side = color;

    attackers[color] = this->getXrayAttacksToSquare(square, color);
    attackers[color ^ 1] = this->getXrayAttacksToSquare(square, static_cast<Color>(color ^ 1));

    while (depth < MAX_SEE_DEPTH && attackers[side])
    {
        int attackerSquare = _tzcnt_u64(attackers[side]);
        attackers[side] &= attackers[side] - 1;

        gains[depth] = value = pieceValue[squarePieceType[attackerSquare]] - value;
        depth++;

        side = side ^ 1;
    }

    while (--depth)
        gains[depth - 1] = std::min(-gains[depth], gains[depth - 1]);

    return gains[0];
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
    this->rookMovement = new uint64_t[102400];
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
    this->bishopMovement = new uint64_t[5248];
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

void ChessEngine::addPawnMoves(MoveList& moveList, const uint64_t mask) const
{
    uint64_t pawns = pieces[activePlayer][PAWN];
    uint64_t allPiecesOnBoard = allPieces[activePlayer] | allPieces[activePlayer ^ 1];
    uint64_t doublePushRank = activePlayer == Color::WHITE ? BitboardGenerator::RANK_2 : BitboardGenerator::RANK_7;
    uint64_t promotionRank = activePlayer == Color::WHITE ? BitboardGenerator::RANK_7 : BitboardGenerator::RANK_2;

    while (pawns)
    {
        // Get the LSB
        int square = _tzcnt_u64(pawns);

        // Bitboard containing the currently processed pawn
        uint64_t singlePawnBitboard = 1ULL << square;

        // Add pawn pushes
        if (!(pawnPushes[activePlayer][square] & allPiecesOnBoard))
        {
            int pushSquare = _tzcnt_u64(pawnPushes[activePlayer][square]);

            if (pawnPushes[activePlayer][square] & mask)
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
                if (!(pawnPushes[activePlayer][pushSquare] & allPiecesOnBoard) && (pawnPushes[activePlayer][pushSquare] & mask))
                    moveList.add(Move(square, _tzcnt_u64(pawnPushes[activePlayer][pushSquare])));
            }
        }

        // Add pawn attacks
        uint64_t successfulAttacks = (pawnAttacks[activePlayer][square] & (allPieces[activePlayer ^ 1] & ~pieces[activePlayer ^ 1][KING])) & mask; // The enemy king can not be captured
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
        if (pawnAttacks[activePlayer][square] & enPassantTargetBitboard)
        {
            // Add en passant attack
            moveList.add(Move(square, _tzcnt_u64(enPassantTargetBitboard), Move::MoveType::EN_PASSANT));
        }

        // Remove the LSB
        pawns &= pawns - 1;
    }
}

void ChessEngine::addKnightMoves(MoveList& moveList, const uint64_t mask) const
{
    uint64_t knights = pieces[activePlayer][KNIGHT];

    while (knights)
    {
        // Get the LSB
        int square = _tzcnt_u64(knights);

        // Add knight moves (the enemy king can not be captured)
        uint64_t possibleMoves = knightMovement[square] & ~allPieces[activePlayer] & ~pieces[activePlayer ^ 1][KING] & mask;
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

void ChessEngine::addKingMoves(MoveList& movelist) const
{
    uint64_t king = pieces[activePlayer][KING];

    while (king)
    {
        // Get the LSB
        int square = _tzcnt_u64(king);

        // Add king moves (enemy king can not be captured)
        uint64_t possibleMoves = kingMovement[square] & ~allPieces[activePlayer] & ~pieces[activePlayer ^ 1][KING];
        while (possibleMoves)
        {
            int attackedSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(square, attackedSquare));
            possibleMoves &= possibleMoves - 1;
        }

        // TODO Check attacks with an attack bitboard
        if (activePlayer == Color::WHITE)
        {
            if (castlingRights & whiteCastleQueenSide)
            {
                if (!(squaresBetween[0][4] & (allPieces[activePlayer] | allPieces[activePlayer ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[1][4] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(activePlayer ^ 1)))
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
                if (!(squaresBetween[7][4] & (allPieces[activePlayer] | allPieces[activePlayer ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[7][4] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(activePlayer ^ 1)))
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
        else if (activePlayer == Color::BLACK)
        {
            if (castlingRights & blackCastleQueenSide)
            {
                if (!(squaresBetween[56][60] & (allPieces[activePlayer] | allPieces[activePlayer ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[57][60] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(activePlayer ^ 1)))
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
                if (!(squaresBetween[63][60] & (allPieces[activePlayer] | allPieces[activePlayer ^ 1])))
                {
                    bool canCastle = true;

                    // Check the squares between the rook and the king (king included) for attacks
                    uint64_t castleBitboard = squaresBetween[63][60] | (1ULL << square);
                    while (castleBitboard)
                    {
                        int castleSquare = _tzcnt_u64(castleBitboard);
                        if (isAttacked(castleSquare, static_cast<Color>(activePlayer ^ 1)))
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

void ChessEngine::addRookMoves(MoveList& movelist, const uint64_t mask) const
{
    uint64_t rooks = pieces[activePlayer][ROOK];
    uint64_t allPiecesOnBoard = allPieces[activePlayer] | allPieces[activePlayer ^ 1];

    while (rooks)
    {
        // Get the LSB
        int square = _tzcnt_u64(rooks);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[square], rookOccupancyMask[square])];
        possibleMoves &= ~allPieces[activePlayer]; // Remove the pieces of the same color from the attack set
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

void ChessEngine::addBishopMoves(MoveList& movelist, const uint64_t mask) const
{
    uint64_t bishops = pieces[activePlayer][BISHOP];
    uint64_t allPiecesOnBoard = allPieces[activePlayer] | allPieces[activePlayer ^ 1];

    while (bishops)
    {
        // Get the LSB
        int square = _tzcnt_u64(bishops);

        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[square], bishopOccupancyMask[square])];
        possibleMoves &= ~allPieces[activePlayer]; // Remove the pieces of the same color from the attack set
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

void ChessEngine::addQueenMoves(MoveList& movelist, const uint64_t mask) const
{
    uint64_t queens = pieces[activePlayer][QUEEN];
    uint64_t allPiecesOnBoard = allPieces[activePlayer] | allPieces[activePlayer ^ 1];

    while (queens)
    {
        // Get the LSB
        int square = _tzcnt_u64(queens);

        // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleRookMoves = rookMovement[rookSquareOffset[square] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[square], rookOccupancyMask[square])];
        // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
        uint64_t possibleBishopMoves = bishopMovement[bishopSquareOffset[square] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[square], bishopOccupancyMask[square])];

        // Combine the rook and bishop moves and remove own pieces from attack set
        uint64_t possibleMoves = (possibleRookMoves | possibleBishopMoves) & ~allPieces[activePlayer];
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

MoveList ChessEngine::getPseudolegalMovesInCheck(const uint64_t attackingSquares) const
{
    MoveList movelist;
    int kingSquare = _tzcnt_u64(pieces[activePlayer][KING]);

    if ((attackingSquares & (attackingSquares - 1)) == 0) // Only one piece attacking the king
    {
        // Generate moves that block the attack or capture the attacker
        int attackingSquare = _tzcnt_u64(attackingSquares);
        uint64_t mask = squaresBetween[kingSquare][attackingSquare] | attackingSquares;

        addKingMoves(movelist);
        addPawnMoves(movelist, mask);
        addKnightMoves(movelist, mask);
        addRookMoves(movelist, mask);
        addBishopMoves(movelist, mask);
        addQueenMoves(movelist, mask);

        return movelist;
    }
    else // Two pieces attacking the king
    {
        // Generate only king moves (no castle)
        uint64_t possibleMoves = kingMovement[kingSquare] & ~allPieces[activePlayer] & ~pieces[activePlayer ^ 1][KING];
        while (possibleMoves)
        {
            int escapeSquare = _tzcnt_u64(possibleMoves);
            movelist.add(Move(kingSquare, escapeSquare));
            possibleMoves &= possibleMoves - 1;
        }
    }

    return movelist;
}

bool ChessEngine::isValid(const Move move)
{
    Color colorToMove = this->activePlayer;

    // Extract move information
    uint8_t toSquare = move.to();
    uint8_t fromSquare = move.from();
    uint64_t toSquareMask = 1ULL << toSquare;
    uint64_t fromSquareMask = 1ULL << fromSquare;
   
    if (!(fromSquareMask & allPieces[colorToMove]))
        return false;

    if (toSquareMask & (allPieces[colorToMove] | pieces[colorToMove ^ 1][KING]))
        return false;

    // Get move type
    Move::MoveType moveType = move.moveType();

    // Get piece type
    PieceType pieceType = squarePieceType[fromSquare];

    if (moveType == Move::MoveType::NORMAL)
    {
        uint64_t allPiecesOnBoard = allPieces[colorToMove] | allPieces[colorToMove ^ 1];
        uint64_t possibleMoves = 0ULL;

        switch (pieceType)
        {
        case PAWN:
        {
            uint64_t doublePushRank = this->activePlayer == Color::WHITE ? BitboardGenerator::RANK_2 : BitboardGenerator::RANK_7;

            if (toSquareMask & pawnPushes[colorToMove][fromSquare]) // Simple pawn push
            {
                if (toSquareMask & allPiecesOnBoard)
                    return false;
            }
            else if ((fromSquareMask & doublePushRank) && toSquareMask == BitboardGenerator::north(BitboardGenerator::north(fromSquareMask))) // Double pawn push
            {
                if ((pawnPushes[colorToMove][fromSquare] | toSquareMask) & allPiecesOnBoard)
                    return false;
            }
            else if (toSquareMask & pawnAttacks[colorToMove][fromSquare]) // Pawn attack
            {
                if (!(toSquareMask & allPieces[colorToMove ^ 1]))
                    return false;
            }
            else // Invalid pawn move
            {
                return false;
            }
        }

            break;

        case KNIGHT:
            // Check if the move is valid
            if (!(knightMovement[fromSquare] & toSquareMask))
                return false;
              
            break;

        case KING:
            // Check if the move is valid
            if (!(kingMovement[fromSquare] & toSquareMask))
                return false;
            
            break;

        case ROOK:
        {
            possibleMoves = rookMovement[rookSquareOffset[fromSquare] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[fromSquare], rookOccupancyMask[fromSquare])];
            if (!(toSquareMask & possibleMoves))
                return false;
        }

            break;

        case BISHOP:
        {
            possibleMoves = bishopMovement[bishopSquareOffset[fromSquare] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[fromSquare], bishopOccupancyMask[fromSquare])];
            if (!(toSquareMask & possibleMoves))
                return false;
        }

            break;

        case QUEEN:
        {
            // Get the rook moves from the pre-generated movement bitboards (use PEXT to hash the current board)
            uint64_t possibleRookMoves = rookMovement[rookSquareOffset[fromSquare] + _pext_u64(allPiecesOnBoard & rookOccupancyMask[fromSquare], rookOccupancyMask[fromSquare])];
            // Get the bishop moves from the pre-generated movement bitboards (use PEXT to hash the current board)
            uint64_t possibleBishopMoves = bishopMovement[bishopSquareOffset[fromSquare] + _pext_u64(allPiecesOnBoard & bishopOccupancyMask[fromSquare], bishopOccupancyMask[fromSquare])];
            // Combine the rook and bishop moves
            possibleMoves = (possibleRookMoves | possibleBishopMoves);
            if (!(toSquareMask & possibleMoves))
                return false;
        }

            break;

        default:
            return false;
        }
    }

    if (moveType == Move::MoveType::PROMOTION)
    {
        // Check if the moving piece is a pawn
        if (pieceType != PAWN)
            return false;

        // Check if the move is a promotion
        uint64_t promotionRank = this->activePlayer == Color::WHITE ? BitboardGenerator::RANK_7 : BitboardGenerator::RANK_2;
        if (!(fromSquareMask & promotionRank))
            return false;

        // Check if the move is valid
        if (!(toSquareMask & (pawnPushes[colorToMove][fromSquare] | pawnAttacks[colorToMove][fromSquare])))
            return false;
    }

    if (moveType == Move::MoveType::CASTLE)
    {
        // Check if the moving piece is a king
        if (pieceType != KING)
            return false;

        if (toSquare == 58) // Black queen side
        {
            if (!(this->castlingRights & this->blackCastleQueenSide))
                return false;

            if (!(squaresBetween[56][60] & (allPieces[colorToMove] | allPieces[colorToMove ^ 1])))
            {
                // Check the squares between the rook and the king (king included) for attacks
                uint64_t castleBitboard = squaresBetween[57][61];
                while (castleBitboard)
                {
                    int castleSquare = _tzcnt_u64(castleBitboard);
                    if (isAttacked(castleSquare, static_cast<Color>(colorToMove ^ 1)))
                        return false;

                    castleBitboard &= castleBitboard - 1;
                }
            }
        }
        else if (toSquare == 62) // Black king side
        {
            if (!(this->castlingRights & this->blackCastleKingSide))
                return false;

            if (!(squaresBetween[60][63] & (allPieces[colorToMove] | allPieces[colorToMove ^ 1])))
            {
                // Check the squares between the rook and the king (king included) for attacks
                uint64_t castleBitboard = squaresBetween[59][63];
                while (castleBitboard)
                {
                    int castleSquare = _tzcnt_u64(castleBitboard);
                    if (isAttacked(castleSquare, static_cast<Color>(colorToMove ^ 1)))
                        return false;

                    castleBitboard &= castleBitboard - 1;
                }
            }
        }
        else if (toSquare == 2) // White queen side
        {
            if (!(this->castlingRights & this->whiteCastleQueenSide))
                return false;

            if (!(squaresBetween[0][4] & (allPieces[colorToMove] | allPieces[colorToMove ^ 1])))
            {
                // Check the squares between the rook and the king (king included) for attacks
                uint64_t castleBitboard = squaresBetween[1][5];
                while (castleBitboard)
                {
                    int castleSquare = _tzcnt_u64(castleBitboard);
                    if (isAttacked(castleSquare, static_cast<Color>(colorToMove ^ 1)))
                        return false;

                    castleBitboard &= castleBitboard - 1;
                }
            }
        }
        else if (toSquare == 6) // White king side
        {
            if (!(this->castlingRights & this->whiteCastleKingSide))
                return false;

            if (!(squaresBetween[4][7] & (allPieces[colorToMove] | allPieces[colorToMove ^ 1])))
            {
                // Check the squares between the rook and the king (king included) for attacks
                uint64_t castleBitboard = squaresBetween[3][7];
                while (castleBitboard)
                {
                    int castleSquare = _tzcnt_u64(castleBitboard);
                    if (isAttacked(castleSquare, static_cast<Color>(colorToMove ^ 1)))
                        return false;

                    castleBitboard &= castleBitboard - 1;
                }
            }
        }
        else // Invalid castle
        {
            return false;
        }
    }

    if (moveType == Move::MoveType::EN_PASSANT)
    {
        // Check if the moving piece is a pawn
        if (pieceType != PAWN)
            return false;

        const uint64_t EN_PASSANT_RANK = this->activePlayer == Color::WHITE ? BitboardGenerator::RANK_5 : BitboardGenerator::RANK_4;
        if (!(fromSquareMask & EN_PASSANT_RANK))
            return false;

        // Check if the en passant is possible in the current state of the board
        if (!(this->enPassantTargetBitboard & toSquareMask))
            return false;
    }

    // Check if the move is legal
    makeMove(move);
    bool isLegal = !isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1));
    undoMove();

    return isLegal;
}

bool ChessEngine::compareMoves(const Move firstMove, const Move secondMove) const
{
    int firstTo = firstMove.to();
    int secondTo = secondMove.to();

    if (pieceValue[squarePieceType[firstTo]] && pieceValue[squarePieceType[secondTo]]) // Both moves are captures
    {
        if (pieceValue[squarePieceType[firstTo]] > pieceValue[squarePieceType[secondTo]])
            return true;

        if (pieceValue[squarePieceType[firstTo]] == pieceValue[squarePieceType[secondTo]])
            if (pieceValue[squarePieceType[firstMove.from()]] < pieceValue[squarePieceType[secondMove.from()]])
                return true;
    }
    else if (pieceValue[squarePieceType[firstTo]]) // Only the first move is a capture
    {
        return true;
    }
    else if (pieceValue[squarePieceType[secondTo]]) // Only the second move is a capture
    {
        return false;
    }
    else
    {
        // Use killer move heuristic
        bool isFirstMoveKiller = (firstMove == killerMoves[currentPly][0] || firstMove == killerMoves[currentPly][1]);
        bool isSecondMoveKiller = (secondMove == killerMoves[currentPly][0] || secondMove == killerMoves[currentPly][1]);
        if (isFirstMoveKiller && isSecondMoveKiller)
            return historyTable[activePlayer][firstMove.from()][firstMove.to()] > historyTable[activePlayer][secondMove.from()][secondMove.to()];
        if (isFirstMoveKiller)
            return true;
        if (isSecondMoveKiller)
            return false;

        // Use history heuristic
        return historyTable[activePlayer][firstMove.from()][firstMove.to()] > historyTable[activePlayer][secondMove.from()][secondMove.to()];
    }

    return false;
}

int ChessEngine::assignScore(const Move move) const
{
    if (pieceValue[squarePieceType[move.to()]])
    {
        int seeScore = this->SEE(move.to(), activePlayer);
        if (seeScore >= 0)
            return MAX_HISTORY_VALUE + 10000 + seeScore;
        return 0;
    }
    else if (move == killerMoves[currentPly][0] || move == killerMoves[currentPly][1])
        return MAX_HISTORY_VALUE + 10000;
    else
        return historyTable[activePlayer][move.from()][move.to()];
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

ChessEngine::SearchResult ChessEngine::minimax(int alpha, int beta, const int depth, const int ply)
{
    numberOfNodesVisited++;

    if (this->stopSearch)
        return SearchResult();

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - this->searchStartTime).count();
    if (duration >= this->timeLimitInMilliseconds)
    {
        this->stopSearch = true;
        return SearchResult();
    }

    // 50 moves rule
    if (!this->isAtRoot && this->halfmoveClock >= 50)
        return SearchResult(0);

    // Threefold repetition rule
    if (!this->isAtRoot && this->halfmoveClock >= 8)
    {
        int repetitions = 0;
        for (int start = std::max(0, this->previousPositionsSize - 1 - this->halfmoveClock), i = this->previousPositionsSize - 3; i >= start; i -= 2)
            if (this->previousPositions[i] == this->boardZobristHash)
            {
                repetitions++;
                if (repetitions == 2)
                    return SearchResult(0);
            }
    }

    const Color colorToMove = this->activePlayer;

    // Check the transposition table entry
    int TTIndex = this->boardZobristHash & (this->transpositionTableSize - 1);
    if (transpositionTable[TTIndex].zobristHash == this->boardZobristHash && transpositionTable[TTIndex].depth >= depth)
    {
        if (transpositionTable[TTIndex].nodeType == NodeType::EXACT)
            if (isValid(transpositionTable[TTIndex].move))
                return SearchResult(transpositionTable[TTIndex].move, transpositionTable[TTIndex].score);

        if (colorToMove == Color::WHITE)
        {
            if (transpositionTable[TTIndex].nodeType == NodeType::LOWER_BOUND && transpositionTable[TTIndex].score >= beta)
                if (isValid(transpositionTable[TTIndex].move))
                    return SearchResult(transpositionTable[TTIndex].move, transpositionTable[TTIndex].score);

            if (transpositionTable[TTIndex].nodeType == NodeType::UPPER_BOUND && transpositionTable[TTIndex].score < alpha)
                if (isValid(transpositionTable[TTIndex].move))
                    return SearchResult(transpositionTable[TTIndex].move, transpositionTable[TTIndex].score);
        }
        else
        {
            if (transpositionTable[TTIndex].nodeType == NodeType::LOWER_BOUND && transpositionTable[TTIndex].score <= alpha)
                if (isValid(transpositionTable[TTIndex].move))
                    return SearchResult(transpositionTable[TTIndex].move, transpositionTable[TTIndex].score);

            if (transpositionTable[TTIndex].nodeType == NodeType::UPPER_BOUND && transpositionTable[TTIndex].score > beta)
                if (isValid(transpositionTable[TTIndex].move))
                    return SearchResult(transpositionTable[TTIndex].move, transpositionTable[TTIndex].score);
        }
    }

    if (depth == 0)
        return SearchResult(evaluate());

    // Store the current ply
    this->currentPly = ply;

    uint64_t squaresAttackingKing = getAttacksBitboard(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1));

    if (!this->isAtRoot && !squaresAttackingKing && depth >= NULL_MOVE_DEPTH_THRESHOLD) // Null move pruning
    {
        if ((pieces[colorToMove][PAWN] | pieces[colorToMove][KING]) != allPieces[colorToMove]) // Avoid zugzwang positions
        {
            UndoHelper lastMove = this->undoStack.top();

            if (lastMove.from() != 0 || lastMove.to() != 0) // Avoid consecutive null moves
            {
                makeMove(Move());

                if (colorToMove == Color::WHITE)
                {
                    int score = minimax(beta - 1, beta, depth - NULL_MOVE_DEPTH_REDUCTION - 1, ply + 1).score;
                    if (score >= beta)
                    {
                        undoMove();
                        return SearchResult(beta);
                    }
                }
                else
                {
                    int score = minimax(alpha, alpha + 1, depth - NULL_MOVE_DEPTH_REDUCTION - 1, ply + 1).score;
                    if (score <= alpha)
                    {
                        undoMove();
                        return SearchResult(alpha);
                    }
                }

                undoMove();
            }
        }
    }

    this->isAtRoot = false;

    MoveList moves = squaresAttackingKing != 0ULL ? getPseudolegalMovesInCheck(squaresAttackingKing) : getPseudolegalMoves();
    sortMoves(moves);
    
    if (colorToMove == Color::WHITE)
    {
        const int originalAlpha = alpha;
        SearchResult result(INT_MIN);

        for (int i = 0; i < moves.numberOfMoves; i++)
        {
            makeMove(moves.moves[i]);

            // Check if the move is legal
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            {
                SearchResult moveResult(moves.moves[i], minimax(alpha, beta, depth - 1, ply + 1).score);

                if (moveResult.score > result.score)
                {
                    result = moveResult;

                    if (moveResult.score > alpha)
                        alpha = moveResult.score;
                }
                
                if (moveResult.score >= beta)
                {
                    undoMove();

                    if (!this->stopSearch)
                    {
                        // Store the result in the transposition table
                        transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::LOWER_BOUND);

                        // Update the killer and history tables for non capture moves
                        if (squarePieceType[moves.moves[i].to()] == PieceType::NONE)
                        {
                            updateKillerMoves(moves.moves[i], ply);
                            updateHistoryTable(colorToMove, moves.moves[i], depth);
                        }
                    }

                    return result;
                }
            }

            undoMove();
        }

        if (result.score == INT_MIN) // No legal move found
        {
            if (squaresAttackingKing) // If the king is in check then it is checkmate
                result.score = CHECKMATE_SCORE[colorToMove] + ply;
            else // If the king is not in check them it is stalemate
                result.score = 0;
        }

        if (!this->stopSearch)
        {
            // Store the result in the transposition table
            if (alpha > originalAlpha)
                transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::EXACT);
            else
                transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::UPPER_BOUND);

            // Update the history table for non capture moves
            if (squarePieceType[result.move.to()] == PieceType::NONE)
                updateHistoryTable(colorToMove, result.move, depth);
        }

        return result;
    }
    else
    {
        const int originalBeta = beta;
        SearchResult result(INT_MAX);

        for (int i = 0; i < moves.numberOfMoves; i++)
        {
            makeMove(moves.moves[i]);

            // Check if the move is legal
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            {
                SearchResult moveResult(moves.moves[i], minimax(alpha, beta, depth - 1, ply + 1).score);

                if (moveResult.score < result.score)
                {
                    result = moveResult;

                    if (moveResult.score < beta)
                        beta = moveResult.score;
                }

                if (moveResult.score <= alpha)
                {
                    undoMove();

                    if (!this->stopSearch)
                    {
                        // Store the result in the transposition table
                        transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::LOWER_BOUND);

                        // Update the killer and history tables for non capture moves
                        if (squarePieceType[moves.moves[i].to()] == PieceType::NONE)
                        {
                            updateKillerMoves(moves.moves[i], ply);
                            updateHistoryTable(colorToMove, moves.moves[i], depth);
                        }
                    }

                    return result;
                }
            }

            undoMove();
        }

        if (result.score == INT_MAX) // No legal move found
        {
            if (squaresAttackingKing) // If the king is in check then it is checkmate
                result.score = CHECKMATE_SCORE[colorToMove] - ply;
            else // If the king is not in check them it is stalemate
                result.score = 0;
        }

        if (!this->stopSearch)
        {
            if (beta < originalBeta)
                transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::EXACT);
            else
                transpositionTable[this->boardZobristHash & (this->transpositionTableSize - 1)] = TranspositionTableEntry(this->boardZobristHash, result.move, result.score, depth, NodeType::UPPER_BOUND);

            // Update the history table for non capture moves
            if (squarePieceType[result.move.to()] == PieceType::NONE)
                updateHistoryTable(colorToMove, result.move, depth);
        }

        return result;
    }
}

int ChessEngine::getTimeForSearch() const
{
    int timeForSearch = this->timeRemaining[this->activePlayer] / 40 + this->timeIncrement[this->activePlayer] / 2;

    if (timeForSearch > this->timeRemaining[this->activePlayer])
        return timeForSearch - this->timeRemaining[this->activePlayer];

    return timeForSearch;
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

Move ChessEngine::getMoveFromString(const std::string moveString) const
{
    // Check string size
    if (moveString.size() < 4 || moveString.size() > 5)
        throw std::invalid_argument("The given string does not resprect UCI rules (String too short or too long).");

    // Check files
    if (!('a' <= moveString[0] && moveString[0] <= 'h') || !('a' <= moveString[2] && moveString[2] <= 'h'))
        throw std::invalid_argument("The given string does not resprect UCI rules (Files not represented correctly).");

    // Check ranks
    if (!('1' <= moveString[1] && moveString[1] <= '9') || !('1' <= moveString[3] && moveString[3] <= '9'))
        throw std::invalid_argument("The given string does not resprect UCI rules (Ranks not represented correctly).");

    // Check promotion type
    if (moveString.size() == 5 && (moveString[4] != 'q' && moveString[4] != 'n' && moveString[4] != 'r' && moveString[4] != 'b'))
        throw std::invalid_argument("The given string does not resprect UCI rules (Promotion type not represented correctly).");

    int fromRank = moveString[1] - '1';
    int fromFile = moveString[0] - 'a';
    int fromSquare = 8 * fromRank + fromFile;

    int toRank = moveString[3] - '1';
    int toFile = moveString[2] - 'a';
    int toSquare = 8 * toRank + toFile;

    if (moveString.size() == 4) // Normal move
    {
        Move::MoveType movetype = Move::MoveType::NORMAL;

        // Check if the move is a castle
        if (this->squarePieceType[fromSquare] == PieceType::KING && this->squaresBetween[fromSquare][toSquare])
            movetype = Move::MoveType::CASTLE;

        // Check if the move is an en passant
        if (this->squarePieceType[fromSquare] == PieceType::PAWN && ((1ULL << toSquare) & this->enPassantTargetBitboard) != 0ULL)
            movetype = Move::MoveType::EN_PASSANT;

        return Move(fromSquare, toSquare, movetype);
    }
    else if (moveString.size() == 5) // Promotion move
    {
        Move::PromotionPiece promotion = Move::PromotionPiece::QUEEN;
        switch (moveString[4])
        {
        case 'q':
            promotion = Move::PromotionPiece::QUEEN;
            break;
        case 'n':
            promotion = Move::PromotionPiece::KNIGHT;
            break;
        case 'r':
            promotion = Move::PromotionPiece::ROOK;
            break;
        case 'b':
            promotion = Move::PromotionPiece::BISHOP;
            break;
        }

        return Move(fromSquare, toSquare, Move::MoveType::PROMOTION, promotion);
    }
}

MoveList ChessEngine::getPseudolegalMoves() const
{
    MoveList moveList;

    addPawnMoves(moveList);
    addKnightMoves(moveList);
    addKingMoves(moveList);
    addRookMoves(moveList);
    addBishopMoves(moveList);
    addQueenMoves(moveList);

    return moveList;
}

MoveList ChessEngine::getLegalMoves()
{
    const Color colorToMove = this->activePlayer;
    MoveList pseudolegalMoves = getPseudolegalMoves();
    MoveList legalMoves;

    for (int i = 0; i < pseudolegalMoves.numberOfMoves; i++)
    {
        makeMove(pseudolegalMoves.moves[i]);
        if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            legalMoves.add(pseudolegalMoves.moves[i]);
        undoMove();
    }

    return legalMoves;
}

void ChessEngine::makeMove(const Move move)
{
    // Update the fullmove counter
    if (this->activePlayer == Color::BLACK)
        this->fullmoveCounter++;

    if (move.isNull()) // Make a null move
    {
        undoStack.push(UndoHelper(0, 0, castlingRights, enPassantTargetBitboard, halfmoveClock));

        // Update the halfmove clock
        this->halfmoveClock++;
        
        // Update zobrist hash for en passant target square
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        // Store the zobrist hash of the position in the previous positions array
        this->previousPositions[this->previousPositionsSize] = this->boardZobristHash;
        this->previousPositionsSize++;

        return;
    }

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
        pieces[activePlayer][movingPieceType] ^= fromSquareMask;
        pieces[activePlayer][movingPieceType] ^= toSquareMask;
        allPieces[activePlayer] ^= fromSquareMask;
        allPieces[activePlayer] ^= toSquareMask;
        // Update zobrist hash for the moving piece
        boardZobristHash ^= pieceZobristHash[movingPieceType][fromSquare];
        boardZobristHash ^= pieceZobristHash[movingPieceType][toSquare];

        // Find captured piece type
        PieceType capturedPieceType = squarePieceType[toSquare];
        
        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, halfmoveClock, moveType, capturedPieceType));

        // Update zobrist hash for en passant target square
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Update the halfmove clock
        halfmoveClock++;

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[activePlayer ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[activePlayer ^ 1] ^= toSquareMask;

            // Update zobrist hash for captured piece
            boardZobristHash ^= pieceZobristHash[capturedPieceType][toSquare];

            if (capturedPieceType == PieceType::ROOK)
            {
                switch (toSquare)
                {
                case 0:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~whiteCastleQueenSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 7:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~whiteCastleKingSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 56:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~blackCastleQueenSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 63:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~blackCastleKingSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                }
            }

            // Reset the halfmove clock on a capture
            this->halfmoveClock = 0;
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = movingPieceType;

        // Update castling rights
        if (movingPieceType == KING)
        {
            if (activePlayer == Color::WHITE)
            {
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            }
            else if (activePlayer == Color::BLACK)
            {
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            }
        }
        else if (movingPieceType == ROOK)
        {
            switch (fromSquare)
            {
            case 0:
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~whiteCastleQueenSide;
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                break;
            case 7:
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~whiteCastleKingSide;
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                break;
            case 56:
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~blackCastleQueenSide;
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                break;
            case 63:
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                castlingRights &= ~blackCastleKingSide;
                boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                break;
            }
        }
        // Update en passant square if a pawn makes a double push
        else if (movingPieceType == PAWN)
        {
            if (squaresBetween[fromSquare][toSquare])
            {
                if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
                enPassantTargetBitboard = squaresBetween[fromSquare][toSquare];
                if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
            }

            // Reset the halfmove clock on a pawn move
            this->halfmoveClock = 0;
        }

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        // Store the zobrist hash of the position in the previous positions array
        this->previousPositions[this->previousPositionsSize] = this->boardZobristHash;
        this->previousPositionsSize++;

        return;
    }

    if (moveType == Move::MoveType::PROMOTION)
    {
        // Get promotion type
        PieceType promotionType = promotionPieceToPieceType[move.promotionPiece()];

        // Move and promote the pawn
        pieces[activePlayer][PAWN] ^= fromSquareMask;
        pieces[activePlayer][promotionType] ^= toSquareMask;
        allPieces[activePlayer] ^= fromSquareMask;
        allPieces[activePlayer] ^= toSquareMask;
        // Update zobrist hash for the promoting pawn
        boardZobristHash ^= pieceZobristHash[PAWN][fromSquare];
        boardZobristHash ^= pieceZobristHash[promotionType][toSquare];

        // Find captured piece type
        PieceType capturedPieceType = squarePieceType[toSquare];

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, halfmoveClock, moveType, capturedPieceType));

        // Update zobrist hash for en passant target square
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Reset the halfmove clock on a pawn promotion
        halfmoveClock = 0;

        // Capture the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[activePlayer ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[activePlayer ^ 1] ^= toSquareMask;

            // Update zobrist hash for captured piece
            boardZobristHash ^= pieceZobristHash[capturedPieceType][toSquare];

            if (capturedPieceType == PieceType::ROOK)
            {
                switch (toSquare)
                {
                case 0:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~whiteCastleQueenSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 7:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~whiteCastleKingSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 56:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~blackCastleQueenSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                case 63:
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    castlingRights &= ~blackCastleKingSide;
                    boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
                    break;
                }
            }
        }

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = promotionType;

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        // Store the zobrist hash of the position in the previous positions array
        this->previousPositions[this->previousPositionsSize] = this->boardZobristHash;
        this->previousPositionsSize++;

        return;
    }

    if (moveType == Move::MoveType::CASTLE)
    {
        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, halfmoveClock, moveType, PieceType::NONE));

        // Update zobrist hash for en passant target square
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Update the halfmove clock
        halfmoveClock++;

        uint8_t rookToSquare = 0, rookFromSquare = 0;
        uint64_t rookToSquareMask = 0, rookFromSquareMask = 0;

        // Identify the castle move
        switch (toSquare)
        {
        case 2: // White queen side
            rookFromSquare = 0, rookToSquare = 3;
            rookFromSquareMask = 1ULL, rookToSquareMask = 1ULL << 3;
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            break;

        case 6: // White king side
            rookFromSquare = 7, rookToSquare = 5;
            rookFromSquareMask = 1ULL << 7, rookToSquareMask = 1ULL << 5;
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            castlingRights &= ~(whiteCastleQueenSide | whiteCastleKingSide);
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            break;

        case 58: // Black queen side
            rookFromSquare = 56, rookToSquare = 59;
            rookFromSquareMask = 1ULL << 56, rookToSquareMask = 1ULL << 59;
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            break;

        case 62: // Black king side
            rookFromSquare = 63, rookToSquare = 61;
            rookFromSquareMask = 1ULL << 63, rookToSquareMask = 1ULL << 61;
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            castlingRights &= ~(blackCastleQueenSide | blackCastleKingSide);
            boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
            break;
        }

        // Move the king
        pieces[activePlayer][KING] ^= fromSquareMask;
        pieces[activePlayer][KING] ^= toSquareMask;
        allPieces[activePlayer] ^= fromSquareMask;
        allPieces[activePlayer] ^= toSquareMask;
        // Update zobrist hash for the moving king
        boardZobristHash ^= pieceZobristHash[KING][fromSquare];
        boardZobristHash ^= pieceZobristHash[KING][toSquare];

        // Move the rook
        pieces[activePlayer][ROOK] ^= rookFromSquareMask;
        pieces[activePlayer][ROOK] ^= rookToSquareMask;
        allPieces[activePlayer] ^= rookFromSquareMask;
        allPieces[activePlayer] ^= rookToSquareMask;
        // Update zobrist hash for the moving rook
        boardZobristHash ^= pieceZobristHash[ROOK][rookFromSquare];
        boardZobristHash ^= pieceZobristHash[ROOK][rookToSquare];

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = PieceType::KING;
        squarePieceType[rookFromSquare] = PieceType::NONE;
        squarePieceType[rookToSquare] = PieceType::ROOK;

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        // Store the zobrist hash of the position in the previous positions array
        this->previousPositions[this->previousPositionsSize] = this->boardZobristHash;
        this->previousPositionsSize++;

        return;
    }

    if (moveType == Move::MoveType::EN_PASSANT)
    {
        // Move the pawn
        pieces[activePlayer][PAWN] ^= fromSquareMask;
        pieces[activePlayer][PAWN] ^= toSquareMask;
        allPieces[activePlayer] ^= fromSquareMask;
        allPieces[activePlayer] ^= toSquareMask;
        // Update zobrist hash for the moving pawn
        boardZobristHash ^= pieceZobristHash[PAWN][fromSquare];
        boardZobristHash ^= pieceZobristHash[PAWN][toSquare];

        // Push the changes to the undo stack
        undoStack.push(UndoHelper(fromSquare, toSquare, castlingRights, enPassantTargetBitboard, halfmoveClock, moveType, PieceType::PAWN));

        // Update zobrist hash for en passant target square
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        // Empty the en passant target bitboard
        enPassantTargetBitboard = 0ULL;

        // Reset the halfmove clock on an en passant move
        halfmoveClock = 0;

        // Compute the captured pawn coordinates
        int capturedPawnSquare = 0;
        uint64_t capturedPawnMask = 0ULL;
        if (activePlayer == Color::WHITE)
        {
            // Go one square down
            capturedPawnSquare = toSquare - 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }
        else if (activePlayer == Color::BLACK)
        {
            // Go one square up
            capturedPawnSquare = toSquare + 8;
            capturedPawnMask = 1ULL << capturedPawnSquare;
        }

        // Capture the enemy pawn
        pieces[activePlayer ^ 1][PAWN] ^= capturedPawnMask;
        allPieces[activePlayer ^ 1] ^= capturedPawnMask;
        // Update the zobrist hash for the captured pawn
        boardZobristHash ^= pieceZobristHash[PAWN][capturedPawnSquare];

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::NONE;
        squarePieceType[toSquare] = PieceType::PAWN;
        squarePieceType[capturedPawnSquare] = PieceType::NONE;

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        // Store the zobrist hash of the position in the previous positions array
        this->previousPositions[this->previousPositionsSize] = this->boardZobristHash;
        this->previousPositionsSize++;

        return;
    }
}

void ChessEngine::undoMove()
{
    // Get the color of the player that made the last move
    const Color colorThatMoved = static_cast<Color>(this->activePlayer ^ 1);

    // Remove the current zobrist hash from the previous positions array
    this->previousPositionsSize--;

    // Update the fullmove counter
    if (colorThatMoved == Color::BLACK)
        this->fullmoveCounter--;

    // Get last move information
    UndoHelper undoHelper = undoStack.top();
    undoStack.pop();

    // Extract move information
    uint8_t toSquare = undoHelper.to();
    uint8_t fromSquare = undoHelper.from();
    uint64_t toSquareMask = 1ULL << toSquare;
    uint64_t fromSquareMask = 1ULL << fromSquare;
    Move::MoveType moveType = undoHelper.moveType();

    if (toSquare == 0 && fromSquare == 0) // Null move
    {
        // Castling rights are not affected by null moves

        // Restore the previous halfmove clock
        this->halfmoveClock = undoHelper.halfmoveClock();

        // Restore the previous en passant target bitboard
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        enPassantTargetBitboard = undoHelper.enPassantBitboard();
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        return;
    }

    if (moveType == Move::MoveType::NORMAL)
    {
        // Find moving piece type
        PieceType movingPieceType = squarePieceType[toSquare];

        // Move the piece back
        pieces[colorThatMoved][movingPieceType] ^= toSquareMask;
        pieces[colorThatMoved][movingPieceType] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;
        // Updated the zobrist hash for the moving piece
        boardZobristHash ^= pieceZobristHash[movingPieceType][toSquare];
        boardZobristHash ^= pieceZobristHash[movingPieceType][fromSquare];

        // Find captured piece type
        PieceType capturedPieceType = static_cast<PieceType>(undoHelper.capturedPieceType());

        // Revert the capture of the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorThatMoved ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorThatMoved ^ 1] ^= toSquareMask;

            // Update the zobrist hash for the captured piece
            boardZobristHash ^= pieceZobristHash[capturedPieceType][toSquare];
        }

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = capturedPieceType;
        squarePieceType[fromSquare] = movingPieceType;

        // Restore the previous castling rights
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
        castlingRights = undoHelper.castlingRights();
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];

        // Restore the previous en passant target bitboard
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        enPassantTargetBitboard = undoHelper.enPassantBitboard();
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];

        // Restore the previous halfmove clock
        this->halfmoveClock = undoHelper.halfmoveClock();

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

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
        // Update the zobrist hash for the pawn that promoted
        boardZobristHash ^= pieceZobristHash[movingPieceType][toSquare];
        boardZobristHash ^= pieceZobristHash[PAWN][fromSquare];

        // Find captured piece type
        PieceType capturedPieceType = static_cast<PieceType>(undoHelper.capturedPieceType());

        // Revert the capture of the enemy piece
        if (capturedPieceType != PieceType::NONE)
        {
            pieces[colorThatMoved ^ 1][capturedPieceType] ^= toSquareMask;
            allPieces[colorThatMoved ^ 1] ^= toSquareMask;

            // Update the zobrist hash for the captured piece
            boardZobristHash ^= pieceZobristHash[capturedPieceType][toSquare];
        }

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = capturedPieceType;
        squarePieceType[fromSquare] = PieceType::PAWN;

        // Restore the previous castling rights
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
        castlingRights = undoHelper.castlingRights();
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];

        // Restore the previous en passant target bitboard
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        enPassantTargetBitboard = undoHelper.enPassantBitboard();
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];

        // Restore the previous halfmove clock
        this->halfmoveClock = undoHelper.halfmoveClock();

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

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
        // Update the zobrist hash for the king that moved
        boardZobristHash ^= pieceZobristHash[KING][toSquare];
        boardZobristHash ^= pieceZobristHash[KING][fromSquare];

        // Move the rook back
        pieces[colorThatMoved][ROOK] ^= rookToSquareMask;
        pieces[colorThatMoved][ROOK] ^= rookFromSquareMask;
        allPieces[colorThatMoved] ^= rookToSquareMask;
        allPieces[colorThatMoved] ^= rookFromSquareMask;
        // Update the zobrist hash for the rook that moved
        boardZobristHash ^= pieceZobristHash[ROOK][rookToSquare];
        boardZobristHash ^= pieceZobristHash[ROOK][rookFromSquare];

        // Update the array that stores piece types for each square
        squarePieceType[toSquare] = PieceType::NONE;
        squarePieceType[fromSquare] = PieceType::KING;
        squarePieceType[rookToSquare] = PieceType::NONE;
        squarePieceType[rookFromSquare] = PieceType::ROOK;

        // Restore the previous castling rights
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
        castlingRights = undoHelper.castlingRights();
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];

        // Restore the previous en passant target bitboard
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        enPassantTargetBitboard = undoHelper.enPassantBitboard();
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];

        // Restore the previous halfmove clock
        this->halfmoveClock = undoHelper.halfmoveClock();

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        return;
    }

    if (moveType == Move::MoveType::EN_PASSANT)
    {
        // Move the pawn back
        pieces[colorThatMoved][PAWN] ^= toSquareMask;
        pieces[colorThatMoved][PAWN] ^= fromSquareMask;
        allPieces[colorThatMoved] ^= toSquareMask;
        allPieces[colorThatMoved] ^= fromSquareMask;
        // Update the zobrist hash for the pawn that moved
        boardZobristHash ^= pieceZobristHash[PAWN][toSquare];
        boardZobristHash ^= pieceZobristHash[PAWN][fromSquare];

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
        // Update the zobrist hash for the pawn that was captured
        boardZobristHash ^= pieceZobristHash[PAWN][capturedPawnSquare];

        // Update the array that stores piece types for each square
        squarePieceType[fromSquare] = PieceType::PAWN;
        squarePieceType[toSquare] = PieceType::NONE;
        squarePieceType[capturedPawnSquare] = PieceType::PAWN;

        // Restore the previous castling rights
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];
        castlingRights = undoHelper.castlingRights();
        boardZobristHash ^= castlingRightsZobristHash[castlingRights & 0xF];

        // Restore the previous en passant target bitboard
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];
        enPassantTargetBitboard = undoHelper.enPassantBitboard();
        if (enPassantTargetBitboard) boardZobristHash ^= enPassantTargetSquareZobristHash[_tzcnt_u64(enPassantTargetBitboard)];

        // Restore the previous halfmove clock
        this->halfmoveClock = undoHelper.halfmoveClock();

        // Change the active player
        this->activePlayer = static_cast<Color>(this->activePlayer ^ 1);
        this->boardZobristHash ^= this->changePlayerZobristHash;

        return;
    }
}

unsigned long long ChessEngine::perft(const int depth)
{
    if (depth == 0)
        return 1;

    const Color colorToMove = this->activePlayer;

    uint64_t squaresAttackingKing = getAttacksBitboard(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1));
    MoveList movelist = squaresAttackingKing ? getPseudolegalMovesInCheck(squaresAttackingKing) : getPseudolegalMoves();

    /*if (depth == 1)
    {
        unsigned long long result = 0;
        for (int i = 0; i < movelist.numberOfMoves; i++)
        {
            makeMove(movelist.moves[i], colorToMove);
            if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING])))
                result++;
            undoMove(colorToMove);
        }
        return result;
    }*/
    
    unsigned long long result = 0;

    for (int i = 0; i < movelist.numberOfMoves; i++)
    {
        makeMove(movelist.moves[i]);
        if (!isAttacked(_tzcnt_u64(pieces[colorToMove][KING]), static_cast<Color>(colorToMove ^ 1)))
            result += perft(depth - 1);
        undoMove();
    }

    return result;
}

ChessEngine::SearchResult ChessEngine::getBestMove()
{
    return this->iterativeDeepeningSearch(this->getTimeForSearch());
}

void ChessEngine::stopCurrentSearch()
{
    this->stopSearch = true;
}

void ChessEngine::clearTranspositionTable()
{
    for (int i = 0; i < this->transpositionTableSize; i++)
        transpositionTable[i] = TranspositionTableEntry();
}

void ChessEngine::clearMoveOrderingTables()
{
    this->maxHistoryValueReached = false;
    for (int color = 0; color < 2; color++)
        for (int from = 0; from < 64; from++)
            for (int to = 0; to < 64; to++)
                this->historyTable[color][from][to] = 0;
}

void ChessEngine::setWhiteTime(const int timeInMilliseconds)
{
    this->timeRemaining[WHITE] = timeInMilliseconds;
}

void ChessEngine::setBlackTime(const int timeInMilliseconds)
{
    this->timeRemaining[BLACK] = timeInMilliseconds;
}

void ChessEngine::setWhiteIncrement(const int incrementInMilliseconds)
{
    this->timeIncrement[WHITE] = incrementInMilliseconds;
}

void ChessEngine::setBlackIncrement(const int incrementInMilliseconds)
{
    this->timeIncrement[BLACK] = incrementInMilliseconds;
}

ChessEngine::SearchResult ChessEngine::search(const int depth)
{
    //return negamax(INT_MIN, INT_MAX, depth, colorToMove);
    return minimax(INT_MIN, INT_MAX, depth, 1);
}

ChessEngine::SearchResult ChessEngine::iterativeDeepeningSearch(const int timeLimit)
{
    this->searchStartTime = std::chrono::high_resolution_clock::now();
    this->timeLimitInMilliseconds = timeLimit;
    this->stopSearch = false;
    SearchResult bestMove;

    int oldNumberOfNodesVisited = 1;
    for (int depth = 1; depth < MAX_DEPTH && !this->stopSearch; depth++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        if (this->maxHistoryValueReached)
        {
            this->decayHistoryTable(); // Scale the history table down to avoid overflow
            this->maxHistoryValueReached = false; // Reset the flag
        }
        numberOfNodesVisited = 0;
        this->isAtRoot = true;
        this->clearKillerMoves(); // Clear the killer moves table
        SearchResult result = this->minimax(INT_MIN, INT_MAX, depth, 1);
        auto stop = std::chrono::high_resolution_clock::now();

        /*if (!this->stopSearch)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            auto searchTimeLeft = timeLimit - std::chrono::duration_cast<std::chrono::milliseconds>(stop - this->searchStartTime).count();

            std::cout << "Depth " << depth << " reached in " << duration << "ms. Nodes searched: " <<
                numberOfNodesVisited << ". Branching factor: " << 1.0 * numberOfNodesVisited / oldNumberOfNodesVisited << " |" <<
                " Move: " << result.move.toString() << ", Evaluation: " << result.score << ", Time left: " << searchTimeLeft << "\n";

            oldNumberOfNodesVisited = numberOfNodesVisited;
        }*/

        if (!this->stopSearch)
            bestMove = result;

        auto searchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        auto searchTimeLeft = timeLimit - std::chrono::duration_cast<std::chrono::milliseconds>(stop - this->searchStartTime).count();
        if (searchTimeLeft < searchDuration * 2)
        {
            this->timeRemaining[this->activePlayer] -= (timeLimit - searchTimeLeft);
            return bestMove;
        }
    }

    this->timeRemaining[this->activePlayer] -= timeLimit;

    return bestMove;
}

uint64_t ChessEngine::getZobristHash() const
{
    return this->boardZobristHash;
}

void ChessEngine::initializeTimeLimits()
{
    this->timeRemaining[WHITE] = this->timeRemaining[BLACK] = 60000 * 10;
    this->timeIncrement[WHITE] = this->timeIncrement[BLACK] = 100;
}
