#include "UCI.h"

std::string UCI::identifyCommand(const std::string& commandLine)
{
	std::string command;
	int i = 0;

	// Get rid of spaces
	while (i < commandLine.size() && commandLine[i] == ' ')
		i++;

	// Get the command
	while (i < commandLine.size() && 'a' <= commandLine[i] && commandLine[i] <= 'z')
	{
		command += commandLine[i];
		i++;
	}

	return command;
}

void UCI::handleGo(const std::string& commandLine)
{
	int i = 3;

	while (i < commandLine.size())
	{
		// Get rid of spaces
		while (i < commandLine.size() && commandLine[i] == ' ')
			i++;

		// Get go option
		std::string option;
		while (i < commandLine.size() && 'a' <= commandLine[i] && commandLine[i] <= 'z')
		{
			option += commandLine[i];
			i++;
		}

		// Get rid of spaces
		while (i < commandLine.size() && commandLine[i] == ' ')
			i++;

		// Get option value
		int value = 0;
		while (i < commandLine.size() && '0' <= commandLine[i] && commandLine[i] <= '9')
		{
			value = value * 10 + commandLine[i] - '0';
			i++;
		}

		if (option == "wtime")
			this->chessEngine.setWhiteTime(value);
		else if (option == "btime")
			this->chessEngine.setBlackTime(value);
		else if (option == "winc")
			this->chessEngine.setWhiteIncrement(value);
		else if (option == "binc")
			this->chessEngine.setBlackIncrement(value);
	}

	std::cout << "bestmove " << this->chessEngine.getBestMove().move.toString() << "\n";
}

void UCI::handleUci() const
{
	std::cout << "id name " << ENGINE_NAME << "\n";
	std::cout << "id author " << AUTHOR << "\n";

	std::cout << "uciok\n";
}

void UCI::handleUciNewGame()
{
	this->chessEngine.clearTranspositionTable();
	this->chessEngine.clearMoveOrderingTables();
}

void UCI::handleIsReady() const
{
	std::cout << "readyok" << "\n";
}

void UCI::handlePosition(const std::string& commandLine)
{
	int i = 8;

	// Get rid of spaces
	while (i < commandLine.size() && commandLine[i] == ' ')
		i++;

	// Get the position type
	std::string positionType;
	while (i < commandLine.size() && 'a' <= commandLine[i] && commandLine[i] <= 'z')
	{
		positionType += commandLine[i];
		i++;
	}

	if (positionType == "startpos")
	{
		chessEngine.loadFENPosition(STARTING_POSITION);

		// Get rid of spaces
		while (i < commandLine.size() && commandLine[i] == ' ')
			i++;

		// Get position option
		std::string positionOption;
		while (i < commandLine.size() && 'a' <= commandLine[i] && commandLine[i] <= 'z')
		{
			positionOption += commandLine[i];
			i++;
		}

		if (positionOption == "moves")
		{
			while (i < commandLine.size())
			{
				// Get rid of spaces
				while (i < commandLine.size() && commandLine[i] == ' ')
					i++;

				// Get position option
				std::string moveString;
				while (i < commandLine.size() && (('a' <= commandLine[i] && commandLine[i] <= 'z') || ('0' <= commandLine[i] && commandLine[i] <= '9')))
				{
					moveString += commandLine[i];
					i++;
				}

				if (!moveString.empty())
					this->chessEngine.makeMove(this->chessEngine.getMoveFromString(moveString));
			}
		}

		return;
	}
	else if (positionType == "fen")
	{
		// Get rid of spaces
		while (i < commandLine.size() && commandLine[i] == ' ')
			i++;

		chessEngine.loadFENPosition(commandLine.substr(i));

		// TODO Read moves after FEN argument

		return;
	}
}

void UCI::handleStop()
{
	this->chessEngine.stopCurrentSearch();
}

void UCI::run()
{
	while (true)
	{
		std::string commandLine;
		std::getline(std::cin, commandLine);

		std::string command = identifyCommand(commandLine);
		if (command == "uci")
		{
			this->handleUci();
		}
		else if (command == "ucinewgame")
		{
			this->handleUciNewGame();
		}
		else if (command == "isready")
		{
			this->handleIsReady();
		}
		else if (command == "position")
		{
			this->handlePosition(commandLine);
		}
		else if (command == "go")
		{
			this->handleGo(commandLine);
		}
		else if (command == "stop")
		{
			this->handleStop();
		}
		else if (command == "quit")
		{
			return;
		}
	}
}
