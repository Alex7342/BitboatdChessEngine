#pragma once
#include "ChessEngine.h"
#include <iostream>
#include <string>

const std::string STARTING_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const std::string ENGINE_NAME = "TDIEngine";
const std::string AUTHOR = "Borgovan Alexandru";

class UCI
{
private:
	ChessEngine chessEngine;

	static std::string identifyCommand(const std::string& commandLine);

	void handleGo(const std::string& commandLine);
	void handleUci() const;
	void handleUciNewGame();
	void handleIsReady() const;
	void handlePosition(const std::string& commandLine);
	void handleStop();

public:
	void run();
};

