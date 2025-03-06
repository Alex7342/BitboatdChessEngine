#include "ChessEngine.h"
#include <iostream>

ChessEngine::ChessEngine() {
    initializeBitboards();
}

void ChessEngine::initializeBitboards() {
    // Standard chess starting position for each piece type
    whitePieces[PAWN] = 0x000000000000FF00ULL;
    whitePieces[KNIGHT] = 0x0000000000000042ULL;
    whitePieces[BISHOP] = 0x0000000000000024ULL;
    whitePieces[ROOK] = 0x0000000000000081ULL;
    whitePieces[QUEEN] = 0x0000000000000008ULL;
    whitePieces[KING] = 0x0000000000000010ULL;

    blackPieces[PAWN] = 0x00FF000000000000ULL;
    blackPieces[KNIGHT] = 0x4200000000000000ULL;
    blackPieces[BISHOP] = 0x2400000000000000ULL;
    blackPieces[ROOK] = 0x8100000000000000ULL;
    blackPieces[QUEEN] = 0x0800000000000000ULL;
    blackPieces[KING] = 0x1000000000000000ULL;
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
