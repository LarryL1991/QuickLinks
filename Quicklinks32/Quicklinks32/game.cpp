#include "stdafx.h"
#include "Quicklinks32.h"
#include <string>
#include <chrono>
#include <fstream>

extern const int SCREENW;
extern const int SCREENH;



extern HFONT s_hFont;
extern HFONT big_hFont;
extern bool showGame;

extern inline bool ExistTest(const std::string& name);

int start;

RECT windowRect = { 0, 0, SCREENW, SCREENH };



struct Point
{
	int X, Y;
};

class tetrisGrid
{
public:

	BOOL clearLines; //if true there are lines that need to be cleared and the animation is over.
	BOOL linesToBeCleared; //If true there are lines that need to be cleared, but the animation isn't done yet.
	BOOL swapped; //latch to prevent someone from swapping more than once per dropped block.
	BOOL firstSwap; //needed to handle the storing of the falling block and the pull of the next block to fall.
	BOOL paused;
	BOOL tetrisGameover;
	byte debug;
	byte spinCounter;
	BOOL bBlockMoved;
	BOOL bDrawProjection;

	std::string sUser[10] = { "GLD",
		"SLV",
		"BRZ",
		"AGM",
		"LGM",
		"MVP",
		"ATM",
		"INE",
		"WAD",
		"LZR" };

	std::string sScore[10] = { "0000300000",
		"0000200000",
		"0000150000",
		"0000100000",
		"0000080000",
		"0000065000",
		"0000060000",
		"0000050000",
		"0000040000",
		"0000030000", };

	int iScore[10]; //sScore converted to int type for easy manipulation later.

	RECT grid[25][25]; //array of rectangles that correspond to the blocks within blockType for drawing the blocks on screen.
	int width, height, lOffset, tOffset, size, gap;
	byte blockType[25][25]; //array of block types for color purposes [0][0] is usually = 9 with [1][0]:[10][18] being the play area initially set to = 0
	bool blockStopped; //Set this to true if you want a block to spawn.
	byte fallingBlockType; //The type of block that is falling for color/shape.
	byte savedBlockType; //saves the tetris piece to the saved area.
	byte nextBlockType; //set the next block before the game even starts.
	byte rotationState; //tracks rotation state of fallingBlockType and fallingBlockCoords
	int tick; //ticks up every single update frame until 20. Then all animation happens on the 20th tick or (20th tick - (level/2))

	Point fallingBlockCoords[4]; //How I track collision of falling block.
	Point projectedBlockCoords[4];

	BOOL lineCleared[20]; //array of BOOL's that represent the lines that need to be cleared vertically. It doesn't really work how I intended though...

	BOOL bShowHighscore = false;

	int playerHighscorePos = -1;
	BOOL bPlayerHighscore = false;

	BOOL bInputInitials = false;
	std::string userInitials = "";
	byte initialsPos = 0;

	BOOL failedWrite = false; //if the hs file failed to write this will change to true and the program will attempt to write to the file until it is successful.

	int level;
	int lines;
	int score;

	void Pause()
	{
		if (!bShowHighscore)
		{
			if (!paused)
			{
				paused = true;
			};
		};

	};

	BOOL ValidateHSFile()
	{
		std::ifstream infile;
		infile.open("hs");
		char temp;
		int counter = 0;
		int letterCount = 0;

		do
		{
			infile >> temp;
			temp = temp - 88 - letterCount;
			letterCount++;
			if (letterCount > 30)
				letterCount = 0;


			if (((counter >= 3 && counter <= 12) || //first score (AKA "0-9"
				(counter >= 16 && counter <= 25) || //second score
				(counter >= 29 && counter <= 38) ||
				(counter >= 42 && counter <= 51) ||
				(counter >= 55 && counter <= 64) ||
				(counter >= 68 && counter <= 77) ||
				(counter >= 81 && counter <= 90) ||
				(counter >= 94 && counter <= 103) ||
				(counter >= 107 && counter <= 116) ||
				(counter >= 120 && counter <= 129))) //tenth score
			{
				if ((temp < 48 || temp > 57)) //"0-9"
				{
					infile.close();
					return false; //The character that was read was not "0-9" (130 seems to be a terminating character so don't read it.)
				};
			}
			else //initials which are going to be "A-Z"
			{
				if ((temp < 65 || temp > 90) && (counter != 130)) //"A-Z"
				{
					infile.close();
					return false; //The character that was read was not "A-Z" (130 seems to be a terminating character so don't read it.)
				};
			}

			counter++;
			if (counter > 131)
			{
				infile.close();
				return false; //the HS file is larger than 130 and is invalid
			};
		} while (!infile.eof());

		if (counter < 131)
		{
			infile.close();
			return false; //the HS file is smaller than 130 and is invalid
		};

		infile.close();
		return true;
	};

	void CreateHighscores()
	{
		std::ofstream outfile;
		outfile.open("hs");

		if (outfile.is_open())
		{
			char temp;
			int letterCount = 0;
			for (int i = 0; i <= 9; i = i++) //9 for an array of 10
			{
				for (int k = 0; k <= 2; k++) //2 for a string size of 3 (0, 1, 2)
				{
					temp = sUser[i].at(k);
					temp = temp + 88 + letterCount;
					outfile << temp;
					letterCount++;
					if (letterCount > 30)
						letterCount = 0;
				};
				for (int k = 0; k <= 9; k++)
				{
					temp = sScore[i].at(k);
					temp = temp + 88 + letterCount;
					outfile << temp;
					letterCount++;
					if (letterCount > 30)
						letterCount = 0;
				};
			};
			outfile.close();
			failedWrite = false;
		}
		else
		{
			failedWrite = true;
			MessageBox(0, "Failed to write highscore to hs file. Trying again.", 0, 0);
		}
	};

	void ReadHighscores()
	{
		if (ValidateHSFile())
		{
			int i = 0;
			std::ifstream infile;
			infile.open("hs");

			char myChar;
			char buff[131]; // [0] through [130] only.
			int letterCount = 0;

			for (int k = 0; k <= 130; k++)
			{
				infile >> myChar;
				myChar = myChar - 88 - letterCount; // (- letterCount) to add diversity of the jumbling otherwise all the 0's look the same in hs file.
				letterCount++;
				if (letterCount > 30)
					letterCount = 0;
				buff[k] = myChar;
			};
			buff[130] = '\0'; //string terminating character positioned at the 131st character.


			sUser[0] = { buff[0], buff[1], buff[2] };

			sScore[0] = { buff[3], buff[4], buff[5], buff[6], buff[7], buff[8], buff[9], buff[10], buff[11], buff[12] };
			for (int i = 1; i <= 9; i++)
			{
				sUser[i] = { buff[(12 * i) + i], buff[(12 * i) + i + 1], buff[(12 * i) + i + 2] };

				sScore[i] = { buff[(12 * i) + i + 3], buff[(12 * i) + i + 4], buff[(12 * i) + i + 5], buff[(12 * i) + i + 6], buff[(12 * i) + i + 7],
					buff[(12 * i) + i + 8], buff[(12 * i) + i + 9], buff[(12 * i) + i + 10], buff[(12 * i) + i + 11], buff[(12 * i) + i + 12] };
			};
			infile.close();
		}
		else //the HS file wasn't valid
		{
			MessageBox(0, "The Highscores file was tampered with. Loading defaults...", "Error, nice try...", 0);
			CreateHighscores(); //Load defaults.
		};
	};

	void init(int gridWidth, int gridHeight, int leftOffset, int topOffset, int blockSize, int blockGap)
	{
		width = gridWidth;
		height = gridHeight;
		lOffset = leftOffset;
		tOffset = topOffset;
		size = blockSize;
		gap = blockGap;

		clearLines = false;
		linesToBeCleared = false;
		swapped = false;
		firstSwap = true;
		paused = false;
		tetrisGameover = false;
		debug = 0;
		spinCounter = 0;
		bBlockMoved = false;
		bDrawProjection = false;
		nextBlockType = (rand() % 7) + 1;
		savedBlockType = 0;
		rotationState = 0;
		tick = 0;
		blockStopped = true;
		level = 1;
		lines = 0;
		score = 0;

		if (ExistTest("hs")) //if it's there, load the values from hs file
		{
			ReadHighscores();
		}
		else //create hs file with defaults
		{
			CreateHighscores();
		};

		for (int i = 0; i <= 9; i++)
		{
			iScore[i] = std::stoi(sScore[i], nullptr, 10); //create an array of integers that match what the strings are that are loaded in from the file or program.
		};

		for (int k = 0; k <= 3; k++)
		{
			fallingBlockCoords[k] = { 0,0 };
			projectedBlockCoords[k] = { 0,0 };
		};

		for (int i = 0; i != gridWidth; i++)
		{
			for (int j = 0; j != gridHeight; j++)
			{
				if (i == 0 || i == 11 || j == gridHeight - 1 || i == gridWidth - 1 || (j == 6 && i > 11) || (j == 12 && i > 11) || (j == 15 && i > 11) || (j == 17 && i > 11) || (j == 0 && i > 11)) //this if statement needs to be used to make the border, but all other rects need to be made still.
					blockType[i][j] = 9;
				else
					blockType[i][j] = 0;

				grid[i][j] = { (i * (blockSize + blockGap)) + leftOffset, (j * (blockSize + blockGap)) + topOffset, ((i * (blockSize + blockGap)) + blockSize) + leftOffset, ((j * (blockSize + blockGap)) + blockSize) + topOffset };

				grid[i][j].top = grid[i][j].top - (((blockSize + blockGap) * 4) + blockGap); //set up an area 10x4 above the playzone.
				grid[i][j].bottom = grid[i][j].top + blockSize;
			};
		};
	};

	void showNextBlock(byte x, byte y, bool bNext)
	{
		int block;
		for (int i = x; i <= (x + 4); i++)
			for (int j = (y - 1); j <= (y + 3); j++)
				blockType[i][j] = 0; //set background of nextblockzone to 0

		if (bNext)
		{
			block = nextBlockType;
		}
		else
		{
			block = savedBlockType;
		};

		switch (block)//(fallingBlockType)//(rand() % 8)
		{
		case 1: // |
		{
			blockType[x][y + 1] = block;
			blockType[x + 1][y + 1] = block;
			blockType[x + 2][y + 1] = block;
			blockType[x + 3][y + 1] = block;
			break;
		};

		case 2: // -|
		{
			blockType[x + 1][y] = block;
			blockType[x + 1][y + 1] = block;
			blockType[x + 1][y + 2] = block;
			blockType[x + 2][y + 1] = block;
			break;
		};
		case 3: //L
		{
			blockType[x + 1][y] = block;
			blockType[x + 1][y + 1] = block;
			blockType[x + 1][y + 2] = block;
			blockType[x + 2][y + 2] = block;
			break;
		};
		case 4: //Backwards L AKA _|
		{
			blockType[x + 2][y] = block;
			blockType[x + 2][y + 1] = block;
			blockType[x + 2][y + 2] = block;
			blockType[x + 1][y + 2] = block;
			break;
		};
		case 5: // `:,
		{
			blockType[x + 1][y] = block;
			blockType[x + 1][y + 1] = block;
			blockType[x + 2][y + 1] = block;
			blockType[x + 2][y + 2] = block;
			break;
		};
		case 6: // ,|'
		{
			blockType[x + 1][y + 1] = block;
			blockType[x + 1][y + 2] = block;
			blockType[x + 2][y] = block;
			blockType[x + 2][y + 1] = block;
			break;
		};
		case 7: // [_]
		{
			blockType[x + 1][y + 1] = block;
			blockType[x + 2][y + 1] = block;
			blockType[x + 1][y + 2] = block;
			blockType[x + 2][y + 2] = block;
			break;
		};
		};
	};

	void PlayerHighscore()
	{
		//The following was copied from init.
		//I call this exist test to validate scores before the program goes to
		//check and finally write to the high scores.
		//an issue arises if someone takes too long to input their score.
		//I should use database when I think about it.

		if (ExistTest("hs")) //if it's there, load the values from hs file
		{
			ReadHighscores();
		}
		else //create hs file with defaults
		{
			CreateHighscores();
		};

		//do this so that we can actually check int values...
		for (int i = 0; i <= 9; i++)
		{
			iScore[i] = std::stoi(sScore[i], nullptr, 10); //create an array of integers that match what the strings are that are loaded in from the file or program.
		};

		for (int i = 9; i >= 0; i--) //start low and go high to auto order scores.
		{
			if (score > iScore[i])
			{
				playerHighscorePos = i;
				bPlayerHighscore = true;
				bInputInitials = true;
			};
		};

		if (bPlayerHighscore && bInputInitials)
		{
			//Shift lower scores/initials down to fit the new score/initials
			for (int i = 9; i >= playerHighscorePos; i--)
			{
				sUser[i] = sUser[i - 1];
				sScore[i] = sScore[i - 1];
				iScore[i] = iScore[i - 1];
			};
			sUser[playerHighscorePos] = "";
			iScore[playerHighscorePos] = score;
		};

	};

	BOOL CheckForGameover()
	{
		if (blockType[4][0] != 0 || blockType[5][0] != 0 || blockType[6][0] != 0 || blockType[7][0] != 0)
		{
			tetrisGameover = true;
			bShowHighscore = true;
			//CHECK FOR HIGHSCORE!
			PlayerHighscore();
		};
		return !tetrisGameover;
	};

	void SpawnBlock(bool bSaved, byte type)
	{
		//NEED TO CHECK FOR GAMEOVER

		if (CheckForGameover())
		{
			blockStopped = false;
			if (bSaved) //SpawnBlock called to spawn from saved and there is a saved block.
			{
				savedBlockType = fallingBlockType;
				fallingBlockType = type;
				if (firstSwap)
				{
					nextBlockType = (rand() % 7) + 1;//rand() % 2;//rand() % 8);
					firstSwap = false;
				};
			}
			else
			{
				fallingBlockType = nextBlockType;
				nextBlockType = (rand() % 7) + 1;//rand() % 2;//rand() % 8);
			};


			showNextBlock(13, 2, true); //show next block
			showNextBlock(13, 8, false);//show saved block
			rotationState = 0;
			bDrawProjection = true;

			switch (fallingBlockType)//(fallingBlockType)//(rand() % 8)
			{
			case 1: // |
			{
				for (int k = 0; k <= 3; k++)
				{
					fallingBlockCoords[k].X = 6;
					fallingBlockCoords[k].Y = k - 3;
				};
				break;
			};
			case 2: // -|
			{
				for (int k = 0; k <= 2; k++)
				{
					fallingBlockCoords[k].X = 6;
					fallingBlockCoords[k].Y = k - 2;
				};
				fallingBlockCoords[3] = { 7, -1 };
				break;
			};
			case 3: //L
			{
				for (int k = 0; k <= 2; k++)
				{
					fallingBlockCoords[k].X = 6;
					fallingBlockCoords[k].Y = k - 2;
				};
				fallingBlockCoords[3] = { 7, 0 };
				break;
			};
			case 4: //Backwards L AKA _|
			{
				for (int k = 0; k <= 2; k++)
				{
					fallingBlockCoords[k].X = 6;
					fallingBlockCoords[k].Y = k - 2;
				};
				fallingBlockCoords[3] = { 5, 0 };


				break;
			};
			case 5: // `:,
			{
				fallingBlockCoords[0] = { 5, -2 };
				fallingBlockCoords[1] = { 5, -1 };
				fallingBlockCoords[2] = { 6, -1 };
				fallingBlockCoords[3] = { 6, 0 };
				break;
			};
			case 6: // ,|'
			{
				fallingBlockCoords[0] = { 6, 0 };
				fallingBlockCoords[1] = { 6, -1 };
				fallingBlockCoords[2] = { 7, -1 };
				fallingBlockCoords[3] = { 7, -2 };
				break;
			};
			case 7: // [_]
			{
				fallingBlockCoords[0] = { 5, 0 };
				fallingBlockCoords[1] = { 5, -1 };
				fallingBlockCoords[2] = { 6, 0 };
				fallingBlockCoords[3] = { 6, -1 };
				break;
			};
			};

			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
			};
		};

	};

	int FindLines()
	{
		int numberOfLines = 0;
		int rowCount = 0;
		int lineCount = 0;

		for (int k = 0; k <= 19; k++)
		{
			lineCleared[k] = false;
		};

		for (int j = 0; j <= 18; j++)
		{
			for (int i = 1; i <= 10; i++)
			{
				if (blockType[i][j] > 10)//cycle through the columns in a row
				{
					rowCount++;
					if (rowCount == 10)//10 columns in a row were full.
					{
						lineCleared[j] = true;//the line needs to be cleared.
						rowCount = 0;
					};
				};
			};
			rowCount = 0;
		};

		for (int k = 0; k <= 19; k++)
		{
			if (lineCleared[k])
			{
				numberOfLines = numberOfLines + 1;
				linesToBeCleared = true;
				for (int i = 1; i <= 10; i++)
				{
					blockType[i][k] = 20;
				};
			};
		};

		return numberOfLines;
	};

	void PostScore(int howManyLines)
	{
		lines = lines + howManyLines;
		level = 1 + (lines / 10);
		switch (howManyLines)
		{
		case 1: score = score + (40 * level); break;
		case 2: score = score + (100 * level); break;
		case 3: score = score + (300 * level); break;
		case 4: score = score + (1200 * level); break;
		};
	};

	void IncrementBlock()
	{
		tick++;


		if (tick >= 20)
		{
			if (clearLines) //ERROR HERE IF I CHANGE THIS IT BREAKS THE GAME!!!
			{
				//delete bad rows and shift other rows down.
				for (int k = 18; k >= 0; k--)
				{
					if (blockType[1][k] == 20)
					{
						for (int j = k; j >= 0; j--)
						{
							if (blockType[1][j - 1] > 10 || blockType[1][j - 1] < 1)
							{
								k++;
							};
							for (int i = 1; i <= 10; i++)
							{
								if (blockType[i][j - 1] > 10 || blockType[i][j - 1] < 1)
								{
									blockType[i][j] = blockType[i][j - 1];
									blockType[i][j - 1] = 0;
								};
							};
						};
					};
				};

				clearLines = false; //This changes to true by itself...


				linesToBeCleared = false;
			};
		};

		if ((tick >= (20 - (level / 2))) && (linesToBeCleared == false) && (blockStopped == false))
		{
			if (!linesToBeCleared)
			{


				if (CanMove(VK_DOWN))
				{

					Move(VK_DOWN);
					tick = 0;
				}
				else
				{
					if (tick >= 20) //won't place block until 20 ticks.
					{				//now check if the block was recently spun/moved before placing.
						if (!bBlockMoved || (spinCounter > 5))
						{
							for (int k = 0; k <= 3; k++)
							{
								blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType + 10;
							};

							//check for lines and handle them.
							PostScore(FindLines());

							swapped = false;
							blockStopped = true;
							bDrawProjection = false;
							spinCounter = 0;
						}
						else
						{
							bBlockMoved = false;
							spinCounter++;
						};

						tick = 0;
					};
				};
			};
		};


	};

	bool CanMove(int direction)
	{
		bool freeSpace = false;
		int collisions = 0;

		switch (direction)
		{
		case VK_LEFT:
		{
			for (int k = 0; k <= 3; k++)
			{
				if (blockType[fallingBlockCoords[k].X - 1][fallingBlockCoords[k].Y] > 8) //greater than 8 because the border is 9 and placed blocks are 10+
				{
					collisions++;
				};
			};
			break;
		};
		case VK_RIGHT:
		{
			for (int k = 0; k <= 3; k++)
			{
				if (blockType[fallingBlockCoords[k].X + 1][fallingBlockCoords[k].Y] > 8)
				{
					collisions++;
				};
			};
			break;
		};

		case VK_DOWN:
		{
			for (int k = 0; k <= 3; k++)
			{
				if (blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y + 1] > 8)
				{
					collisions++;
				};
			};
			break;
		};

		default:
		{
			break;
		};
		};

		if (collisions > 0)
			return false;
		else
			return true;
	};

	void RotateBlock(bool direction)
	{
		int canPlace = 0;
		int moveLeft = 0;
		switch (fallingBlockType)
		{
		case 1: // |
		{
			if (rotationState == 0) //vertical | to horizontal --
			{
				//MessageBox(0, 0, 0, 0);

				if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
					blockType[fallingBlockCoords[3].X + 2][fallingBlockCoords[3].Y - 2] > 8)
				{
					//then we can't move
					//MessageBox(0, std::to_string(blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1]).c_str(), 0, 0);

				}
				else
				{

					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y + 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X + 2, fallingBlockCoords[3].Y - 2 };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState++;

				};
			}
			else if (rotationState == 1) //horizontal -- to vertical |
			{
				if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y - 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
					blockType[fallingBlockCoords[3].X - 2][fallingBlockCoords[3].Y + 2] > 8)
				{
					//then we can't move
					//MessageBox(0, std::to_string(blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1]).c_str(), 0, 0);

				}
				else
				{
					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y - 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X - 2, fallingBlockCoords[3].Y + 2 };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState--;
				};
			};

			break;
		};
		case 2: // |-
		{
			switch (rotationState)
			{
			case 0:
			{
				if (direction) //clockwise  |- to ^|^
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X - 1][fallingBlockCoords[3].Y + 1] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X - 1, fallingBlockCoords[3].Y + 1 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 1;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 1:
			{
				if (direction) //clockwise ^|^ to -|
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X - 1][fallingBlockCoords[3].Y - 1] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X - 1, fallingBlockCoords[3].Y - 1 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 2;
					};
				}
				else //counter-clockwise ^|^ to |-
				{

				};
				break;
			};
			case 2:
			{
				if (direction) //clockwise -| to _|_
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X + 1][fallingBlockCoords[3].Y - 1] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X + 1, fallingBlockCoords[3].Y - 1 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 3;
					};
				};
				break;
			};
			case 3:
			{
				if (direction) //clockwise _|_ to |-
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X + 1][fallingBlockCoords[3].Y + 1] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X + 1, fallingBlockCoords[3].Y + 1 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 0;
					};
				};
				break;
			};
			};
			break;
		};
		case 3: // L
		{
			switch (rotationState)
			{
			case 0:
			{
				if (direction) //clockwise  L to |````
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X - 2][fallingBlockCoords[3].Y] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X - 2, fallingBlockCoords[3].Y };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 1;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 1:
			{
				if (direction) //clockwise |```` to 7
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y - 2] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y - 2 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 2;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 2:
			{
				if (direction) //clockwise 7 to __|
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X + 2][fallingBlockCoords[3].Y] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X + 2, fallingBlockCoords[3].Y };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 3;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 3:
			{
				if (direction) //clockwise __| to L
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y + 2] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y + 2 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 0;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			};
			break;
		};
		case 4: //Backwards L AKA _|
		{
			switch (rotationState)
			{
			case 0:
			{
				if (direction) //clockwise  _| to |____
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y - 2] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y - 2 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 1;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 1:
			{
				if (direction) //clockwise  |____ to |``
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y + 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
						blockType[fallingBlockCoords[3].X + 2][fallingBlockCoords[3].Y] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y + 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X + 2, fallingBlockCoords[3].Y };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 2;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 2:
			{
				if (direction) //clockwise |`` to ````|
				{
					if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y + 2] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y + 2 };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 3;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			case 3:
			{
				if (direction) //clockwise ````| to _|
				{
					if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y - 1] > 8 ||
						blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
						blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
						blockType[fallingBlockCoords[3].X - 2][fallingBlockCoords[3].Y] > 8)
					{
						//can't move
					}
					else
					{
						//can move.
						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						};

						fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y - 1 };
						fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
						fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
						fallingBlockCoords[3] = { fallingBlockCoords[3].X - 2, fallingBlockCoords[3].Y };

						for (int k = 0; k <= 3; k++)
						{
							blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
						};
						rotationState = 0;
					};
				}
				else //counter-clockwise |- to _|_
				{

					//Need code here in the future.
				};
				break;
			};
			};
			break;
		};
		case 5: // `:,
		{
			switch (rotationState)
			{
			case 0: //clockwise  `|, to ,-`
			{
				if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y + 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
					blockType[fallingBlockCoords[3].X - 2][fallingBlockCoords[3].Y] > 8)
				{
					//can't move
				}
				else
				{
					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y + 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X - 2, fallingBlockCoords[3].Y };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState = 1;
				};
				break;
			};
			case 1: //clockwise ,-` to `|,
			{
				if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y - 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
					blockType[fallingBlockCoords[3].X + 2][fallingBlockCoords[3].Y] > 8)
				{
					//can't move
				}
				else
				{
					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y - 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X + 2, fallingBlockCoords[3].Y };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState = 0;
				};
				break;
			};
			};
			break;
		};
		case 6: // ,|'
		{
			switch (rotationState)
			{
			case 0:
			{
				if (blockType[fallingBlockCoords[0].X - 1][fallingBlockCoords[0].Y - 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X - 1][fallingBlockCoords[2].Y + 1] > 8 ||
					blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y + 2] > 8)
				{
					//can't move
				}
				else
				{
					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X - 1, fallingBlockCoords[0].Y - 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X - 1, fallingBlockCoords[2].Y + 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y + 2 };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState = 1;
				};
				break;
			};
			case 1:
			{
				if (blockType[fallingBlockCoords[0].X + 1][fallingBlockCoords[0].Y + 1] > 8 ||
					blockType[fallingBlockCoords[1].X][fallingBlockCoords[1].Y] > 8 ||
					blockType[fallingBlockCoords[2].X + 1][fallingBlockCoords[2].Y - 1] > 8 ||
					blockType[fallingBlockCoords[3].X][fallingBlockCoords[3].Y - 2] > 8)
				{
					//can't move
				}
				else
				{
					//can move.
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
					};

					fallingBlockCoords[0] = { fallingBlockCoords[0].X + 1, fallingBlockCoords[0].Y + 1 };
					fallingBlockCoords[1] = { fallingBlockCoords[1].X, fallingBlockCoords[1].Y };
					fallingBlockCoords[2] = { fallingBlockCoords[2].X + 1, fallingBlockCoords[2].Y - 1 };
					fallingBlockCoords[3] = { fallingBlockCoords[3].X, fallingBlockCoords[3].Y - 2 };

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};
					rotationState = 0;
				};
				break;
			};
			};
			break;
		};
		};
	};

	void Move(int direction)
	{
		switch (direction)
		{
		case VK_LEFT:
		{
			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
			};

			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X - 1][fallingBlockCoords[k].Y] = fallingBlockType;
				fallingBlockCoords[k] = { fallingBlockCoords[k].X - 1, fallingBlockCoords[k].Y };
			};
			break;
		};
		case VK_RIGHT:
		{
			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
			};

			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X + 1][fallingBlockCoords[k].Y] = fallingBlockType;
				fallingBlockCoords[k] = { fallingBlockCoords[k].X + 1, fallingBlockCoords[k].Y };
			};
			break;
		};
		case VK_DOWN:
		{
			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
			};

			for (int k = 0; k <= 3; k++)
			{
				blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y + 1] = fallingBlockType;
				fallingBlockCoords[k] = { fallingBlockCoords[k].X, fallingBlockCoords[k].Y + 1 };
			};
			break;
		};
		default:
		{
			break;
		};
		};
	};

	void SaveBlock()
	{
		swapped = true;
		for (int k = 0; k <= 3; k++)
			blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0; //clear falling pieces visually.
		if (firstSwap)
		{
			savedBlockType = fallingBlockType;
			SpawnBlock(true, nextBlockType);
		}
		else
		{
			SpawnBlock(true, savedBlockType);
		};

	};

	void ProjectBlock()
	{
		bool bFoundStop = false;
		int collisions = 0;




		for (int k = 0; k <= 3; k++)
		{
			projectedBlockCoords[k].X = fallingBlockCoords[k].X;
			projectedBlockCoords[k].Y = fallingBlockCoords[k].Y;
		};


		for (int j = 0; j <= 18; j++)
		{
			for (int k = 0; k <= 3; k++)
			{
				if (blockType[projectedBlockCoords[k].X][projectedBlockCoords[k].Y + 1] > 8)
				{
					collisions++;
				};
			};

			if (collisions > 0)
			{
				bFoundStop = true;
			};


			for (int k = 0; k <= 3; k++)
			{
				if (!bFoundStop)
				{
					projectedBlockCoords[k].Y = projectedBlockCoords[k].Y + 1;

				};
			};
		};

	};

	void EnterHighscore()
	{


		//place new initials into string
		sUser[playerHighscorePos] = userInitials; //Might have to format this for the hs file...

		sScore[playerHighscorePos] = std::to_string(score); //fills the score in.
		int len = sScore[playerHighscorePos].length(); //store the length of the score string for the next loop.
		for (int i = 0; i < 10 - len; i++)
		{
			sScore[playerHighscorePos].replace(0, 0, "0"); //add "0" in front for hs file formatting until the string length is 10.
		};

		//write to hs "encrypted"
		CreateHighscores();

	};

	void UserInput(int direction)
	{
		if (!bInputInitials) //Only process this extra stuff when the user is not inputting their initials into highscores.
		{
			switch (direction)
			{
			case VK_LEFT:
			{

				if (CanMove(direction) && !paused && !tetrisGameover && !blockStopped)
				{
					bBlockMoved = true;
					Move(direction);
				};
				break;
			};
			case VK_RIGHT:
			{
				if (CanMove(direction) && !paused && !tetrisGameover && !blockStopped)
				{
					bBlockMoved = true;
					Move(direction);
				};
				break;
			};
			case VK_UP://up
			{
				if (!paused && !tetrisGameover && !blockStopped)
				{
					bBlockMoved = true;
					RotateBlock(true);
				};

				break;
			};
			case VK_DOWN://down
			{
				if (CanMove(direction) && !paused && !tetrisGameover && !blockStopped)
				{
					Move(direction);
				};
				break;
			};
			case VK_CONTROL: //Saves the current piece. Can only be done once per spawn.
			{
				if (!swapped && !paused && !tetrisGameover && !blockStopped)
				{
					SaveBlock();
				};
				//blockStopped = true;
				break;
			};
			case VK_SPACE:
			{
				//Slam
				if (!paused && !tetrisGameover && !blockStopped)
				{
					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = 0;
						fallingBlockCoords[k].X = projectedBlockCoords[k].X;
						fallingBlockCoords[k].Y = projectedBlockCoords[k].Y;
					};

					for (int k = 0; k <= 3; k++)
					{
						blockType[fallingBlockCoords[k].X][fallingBlockCoords[k].Y] = fallingBlockType;
					};

					tick = 20;
					spinCounter = 6;
				};
				break;
			};
			case VK_ESCAPE:
			{
				if (!tetrisGameover)
					paused = true;
				showGame = false;
				break;
			};
			case 0x50: //p
			{
				if (!tetrisGameover)
				{
					if (paused)
					{
						paused = false;
						if (bShowHighscore)
							bShowHighscore = false;
					}
					else
						paused = true;
				}
				else
				{
					if (bShowHighscore)
						bShowHighscore = false;
					//reset playfield
					for (int i = 1; i <= 10; i++)
						for (int j = 0; j <= 18; j++)
							blockType[i][j] = 0;
					lines = 0;
					level = 1;
					score = 0;
					savedBlockType = 0;
					firstSwap = true;
					tetrisGameover = false;
				}
				break;
			};
			case 0x44: //d
			{
				if (debug < 5)
				{
					debug++;
				}
				else
					debug = 0;
				break;
			};
			case 0x48: //h (highscores)
			{
				if (!tetrisGameover)
				{
					if (!paused)
						paused = true;
				}
				else
				{
					//reset playfield
					for (int i = 1; i <= 10; i++)
						for (int j = 0; j <= 18; j++)
							blockType[i][j] = 0;
					lines = 0;
					level = 1;
					score = 0;
					savedBlockType = 0;
					firstSwap = true;
					tetrisGameover = false;
				}
				if (bShowHighscore)
				{
					bShowHighscore = false;
					paused = false;
				}
				else
					bShowHighscore = true;

				break;
			};
			default:
			{
				break;
			};
			};
		}
		else //Input initials here.
		{
			switch (direction)
			{
			case VK_BACK:
			{
				//Code for deleting a character.
				if (initialsPos > 0)
				{
					initialsPos--;
					userInitials = userInitials.replace(initialsPos, 1, "");

				};
				break;
			};
			case VK_RETURN:
			{
				//Code for submitting initials
				if (initialsPos == 3)
				{
					bInputInitials = false;
					EnterHighscore();
					userInitials = "";
					initialsPos = 0;
					//bInputInitials = false;
				}
				break;
			};
			default:
			{
				//Code for entering characters.
				if (direction >= 0x41 && direction <= 0x5a) //Check for keys A-Z
				{
					if (initialsPos < 3)
					{
						char initialsTemp;

						initialsTemp = direction;

						userInitials.push_back(initialsTemp);

						initialsPos++;
					};
				};
			};
			};
		};
	};

	VOID DrawProjection(HDC hdc)
	{
		HBRUSH activeBrush;
		HBRUSH shadowBrush;
		RECT rect;

		switch (fallingBlockType)
		{
		case 1:
		{
			activeBrush = CreateSolidBrush(RGB(0, 0, 200));
			shadowBrush = CreateSolidBrush(RGB(0, 0, 40));
			break;
		};
		case 2:
		{
			activeBrush = CreateSolidBrush(RGB(200, 0, 0));
			shadowBrush = CreateSolidBrush(RGB(35, 0, 0));
			break;
		};
		case 3:
		{
			activeBrush = CreateSolidBrush(RGB(0, 200, 0));
			shadowBrush = CreateSolidBrush(RGB(0, 35, 0));
			break;
		};
		case 4:
		{
			activeBrush = CreateSolidBrush(RGB(200, 200, 0));
			shadowBrush = CreateSolidBrush(RGB(35, 35, 0));
			break;
		};
		case 5:
		{
			activeBrush = CreateSolidBrush(RGB(0, 255, 255));
			shadowBrush = CreateSolidBrush(RGB(0, 35, 35));
			break;
		};
		case 6:
		{
			activeBrush = CreateSolidBrush(RGB(200, 0, 200));
			shadowBrush = CreateSolidBrush(RGB(35, 0, 35));
			break;
		};
		case 7:
		{
			activeBrush = CreateSolidBrush(RGB(150, 75, 0));
			shadowBrush = CreateSolidBrush(RGB(40, 25, 0));
			break;
		};
		default:
		{
			activeBrush = CreateSolidBrush(RGB(255, 255, 255));
			shadowBrush = CreateSolidBrush(RGB(255, 255, 255));
			break;
		};
		};

		for (int k = 0; k <= 3; k++)
		{
			if (blockType[projectedBlockCoords[k].X][projectedBlockCoords[k].Y] == 0)
			{
				FillRect(hdc, &grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y], shadowBrush); //color lightly the whole block projection.

				rect = { grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].left,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].top,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].right,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].top + 2 };

				FillRect(hdc, &rect, activeBrush); //Fill in top of projected blocks

				rect = { grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].left,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].top,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].left + 2,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].bottom };

				FillRect(hdc, &rect, activeBrush); //Fill in left of projected blocks

				rect = { grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].right,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].top,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].right - 2,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].bottom };

				FillRect(hdc, &rect, activeBrush); //Fill in right of projected blocks

				rect = { grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].left,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].bottom - 2,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].right,
					grid[projectedBlockCoords[k].X][projectedBlockCoords[k].Y].bottom };

				FillRect(hdc, &rect, activeBrush); //Fill in bottom of projected blocks
			};
		};

		DeleteObject(activeBrush);
		DeleteObject(shadowBrush);
	};

	VOID DrawBlocks(HDC hdc)
	{
		RECT rect;
		int textheight;
		HBRUSH oneBrush = CreateSolidBrush(RGB(0, 0, 200)); //There has to be a better way to do this.
		HBRUSH twoBrush = CreateSolidBrush(RGB(200, 0, 0));
		HBRUSH threeBrush = CreateSolidBrush(RGB(0, 200, 0));
		HBRUSH fourBrush = CreateSolidBrush(RGB(200, 200, 0));
		HBRUSH fiveBrush = CreateSolidBrush(RGB(0, 255, 255));
		HBRUSH sixBrush = CreateSolidBrush(RGB(200, 0, 200));
		HBRUSH sevenBrush = CreateSolidBrush(RGB(150, 75, 0));
		HBRUSH borderBrush = CreateSolidBrush(RGB(100, 100, 100));
		HBRUSH backBrush = CreateSolidBrush(RGB(0, 0, 0));
		HBRUSH lineBrushA = CreateSolidBrush(RGB(200, 200, 200));
		HBRUSH lineBrushB = CreateSolidBrush(RGB(100, 100, 100));
		HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));


		char scoreboard[100];
		char lineboard[100];
		char levelboard[100];

		char debugString[100];
		char debugString2[100];

		const TCHAR* fontName = _T("Arial");
		long nFontSize = 12;
		LOGFONT logFont = { 0 };
		logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		logFont.lfWeight = FW_BOLD;
		_tcscpy_s(logFont.lfFaceName, fontName);
		s_hFont = CreateFontIndirect(&logFont);

		nFontSize = 28;
		logFont = { 0 };
		logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		logFont.lfWeight = FW_BOLD;
		_tcscpy_s(logFont.lfFaceName, fontName);
		big_hFont = CreateFontIndirect(&logFont);

		SelectObject(hdc, s_hFont);

		for (int i = 0; i != width; i++)
		{
			for (int j = 0; j != height; j++)
			{
				switch (blockType[i][j])
				{
				case 0:
				{
					FillRect(hdc, &grid[i][j], backBrush);
					///////////////////Draw Projected Position/////////////////
					if (bDrawProjection)
					{
						DrawProjection(hdc);
					};
					///////////////////Draw Projected Position/////////////////
					break;
				};
				case 1:
				{
					FillRect(hdc, &grid[i][j], oneBrush);
					break;
				};
				case 2:
				{
					FillRect(hdc, &grid[i][j], twoBrush);
					break;
				};
				case 3:
				{
					FillRect(hdc, &grid[i][j], threeBrush);
					break;
				};
				case 4:
				{
					FillRect(hdc, &grid[i][j], fourBrush);
					break;
				};
				case 5:
				{
					FillRect(hdc, &grid[i][j], fiveBrush);
					break;
				};
				case 6:
				{
					FillRect(hdc, &grid[i][j], sixBrush);
					break;
				};
				case 7:
				{
					FillRect(hdc, &grid[i][j], sevenBrush);
					break;
				};
				case 11:
				{
					FillRect(hdc, &grid[i][j], oneBrush);
					break;
				};
				case 12:
				{
					FillRect(hdc, &grid[i][j], twoBrush);
					break;
				};
				case 13:
				{
					FillRect(hdc, &grid[i][j], threeBrush);
					break;
				};
				case 14:
				{
					FillRect(hdc, &grid[i][j], fourBrush);
					break;
				};
				case 15:
				{
					FillRect(hdc, &grid[i][j], fiveBrush);
					break;
				};
				case 16:
				{
					FillRect(hdc, &grid[i][j], sixBrush);
					break;
				};
				case 17:
				{
					FillRect(hdc, &grid[i][j], sevenBrush);
					break;
				};
				case 20:
				{
					if (tick >= 0 && tick <= 10)
						FillRect(hdc, &grid[i][j], lineBrushA);
					else if (tick > 10 && tick <= 15)
						FillRect(hdc, &grid[i][j], lineBrushB);
					else
					{
						FillRect(hdc, &grid[i][j], backBrush);
						clearLines = true;
						//MessageBox(0, std::to_string(i).c_str(), std::to_string(j).c_str(), 0);
					};
					break;
				};
				case 9:
				{
					FillRect(hdc, &grid[i][j], borderBrush);
					if (i == 0) //left squares.
					{
						rect = { 0, grid[i][j].top, grid[i][j].left - gap, grid[i][j].bottom };
						FillRect(hdc, &rect, borderBrush);
					};
					if (j == 19)//bottom squares.
					{
						rect = { grid[i][j].left, grid[i][j].bottom + gap, grid[i][j].right, SCREENH };
						FillRect(hdc, &rect, borderBrush);
						if (i == 0) //bottom-left square.
						{
							rect = { 0, grid[i][j].bottom + gap, grid[i][j].left - gap, SCREENH };
							FillRect(hdc, &rect, borderBrush);
						};
						if (i == 18)//bottom-right square.
						{
							rect = { grid[i][j].right + gap, grid[i][j].bottom + gap, SCREENW, SCREENH };
							FillRect(hdc, &rect, borderBrush);
						};
					};
					if (i == 18) //right squares.
					{
						rect = { grid[i][j].right + gap, grid[i][j].top, SCREENW, grid[i][j].bottom };
						FillRect(hdc, &rect, borderBrush);
					};
					break;
				};
				};
			};
		};



		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));

		rect = grid[13][1];
		textheight = DrawTextA(hdc, "Next Block:", -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, "Next Block:", -1, &rect, DT_CENTER);

		rect = grid[13][7];
		textheight = DrawTextA(hdc, "Saved Block:", -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, "Saved Block:", -1, &rect, DT_CENTER);

		sprintf(scoreboard, "Score: ");

		rect = grid[12][13];
		textheight = DrawTextA(hdc, scoreboard, -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, scoreboard, -1, &rect, DT_LEFT);

		rect = { grid[12][13].left, grid[12][13].bottom - 1, grid[17][14].right, grid[17][14].top + 1 };
		FillRect(hdc, &rect, borderBrush);

		rect = { grid[12][14].left + 2, grid[12][14].top + 3,  grid[16][14].right, grid[16][14].bottom };
		textheight = DrawTextA(hdc, std::to_string(score).c_str(), -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, std::to_string(score).c_str(), -1, &rect, DT_RIGHT);

		sprintf(levelboard, "Level: ");
		strcat(levelboard, std::to_string(level).c_str());

		rect = grid[12][16];
		textheight = DrawTextA(hdc, levelboard, -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, levelboard, -1, &rect, DT_CENTER);

		sprintf(lineboard, "Lines: ");
		strcat(lineboard, std::to_string(lines).c_str());

		rect = grid[12][18];
		textheight = DrawTextA(hdc, lineboard, -1, &rect, DT_CALCRECT);
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, lineboard, -1, &rect, DT_CENTER);

		if (paused && !bShowHighscore)
		{
			SelectObject(hdc, big_hFont);
			SetBkColor(hdc, RGB(0, 0, 0));
			SetBkMode(hdc, OPAQUE);
			rect = { grid[3][9].left, grid[3][9].top, grid[10][9].right, grid[10][9].bottom };//grid[1][9];
			textheight = DrawTextA(hdc, "PAUSED", -1, &rect, DT_CALCRECT);
			rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
			DrawText(hdc, "PAUSED", -1, &rect, DT_CENTER);
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, s_hFont);
		};

		switch (debug)
		{
		case 1:
		{
			for (int i = 0; i <= 11; i++)
				for (int j = 0; j <= 19; j++)
				{
					rect = grid[i][j];
					DrawText(hdc, std::to_string(blockType[i][j]).c_str(), -1, &rect, DT_CENTER);
				};
			break;
		};


		case 2:
		{
			for (int k = 0; k <= 3; k++)
			{
				rect = grid[fallingBlockCoords[k].X][fallingBlockCoords[k].Y];
				DrawText(hdc, std::to_string(k).c_str(), -1, &rect, DT_CENTER);
			};
			break;
		};

		case 3:
		{
			for (int k = 0; k <= 3; k++)
			{
				//MessageBox(0, std::to_string(&fallingBlockCoords[k].X).c_str(), 0, 0);
				rect = grid[1][10 + k];
				sprintf(debugString, "0x%x", &fallingBlockCoords[k].Y);
				textheight = DrawTextA(hdc, debugString, -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, debugString, -1, &rect, DT_CENTER);

				rect = grid[1][15 + k];
				sprintf(debugString2, "0x%x", &projectedBlockCoords[k].Y);
				//strcat(debugString, std::to_string(fallingBlockCoords[k].Y).c_str());
				textheight = DrawTextA(hdc, debugString2, -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, debugString2, -1, &rect, DT_CENTER);
			};
			break;
		};

		case 4:
		{
			SetTextColor(hdc, RGB(0, 255, 255));
			for (int j = 0; j <= 18; j++)
			{
				rect = grid[0][j];
				DrawText(hdc, std::to_string(j).c_str(), -1, &rect, DT_CENTER);
				rect = grid[18][j];
				DrawText(hdc, std::to_string(j).c_str(), -1, &rect, DT_CENTER);
			};
			rect = grid[18][19];
			DrawText(hdc, std::to_string(19).c_str(), -1, &rect, DT_CENTER);

			SetTextColor(hdc, RGB(255, 0, 0));
			for (int i = 0; i <= 17; i++)
			{
				rect = grid[i][19];
				DrawText(hdc, std::to_string(i).c_str(), -1, &rect, DT_CENTER);
				rect = grid[i][0];
				DrawText(hdc, std::to_string(i).c_str(), -1, &rect, DT_CENTER);
			};
			SetTextColor(hdc, RGB(255, 255, 255));
			break;
		};
		case 5:
		{
			SetBkColor(hdc, RGB(0, 0, 0));
			SetBkMode(hdc, OPAQUE);
			rect = grid[0][19];
			sprintf(debugString, std::to_string(tick).c_str());
			textheight = DrawTextA(hdc, debugString, -1, &rect, DT_CALCRECT);
			rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
			DrawText(hdc, debugString, -1, &rect, DT_CENTER);

			rect = grid[3][19];
			sprintf(debugString, std::to_string(spinCounter).c_str());
			textheight = DrawTextA(hdc, debugString, -1, &rect, DT_CALCRECT);
			rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
			DrawText(hdc, debugString, -1, &rect, DT_CENTER);

			SetBkMode(hdc, TRANSPARENT);
			break;
		};
		};

		if (bShowHighscore)
		{
			rect = { grid[1][0].left, grid[1][0].top, grid[10][18].right, grid[10][18].bottom };
			FillRect(hdc, &rect, backBrush);

			SetBkColor(hdc, RGB(0, 0, 0));
			SetBkMode(hdc, TRANSPARENT);

			SelectObject(hdc, big_hFont);
			rect = { grid[3][0].left - 3, grid[3][0].top, grid[10][4].right, grid[10][4].bottom };
			textheight = DrawTextA(hdc, "HIGH\nSCORES", -1, &rect, DT_CALCRECT);
			rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
			DrawText(hdc, "HIGH\nSCORES", -1, &rect, DT_CENTER);
			SelectObject(hdc, s_hFont);

			rect = { grid[1][2].left, grid[1][2].bottom + 12, grid[10][2].right, grid[10][2].bottom + 17 }; //Line under "High\nScores"
			FillRect(hdc, &rect, borderBrush);

			rect = { grid[1][14].left, grid[1][14].top, grid[10][14].right, grid[10][14].top + 5 }; //Line under #10 score.
			FillRect(hdc, &rect, borderBrush);

			if (bInputInitials)
			{
				rect = { grid[1][4 + playerHighscorePos].left, grid[1][4 + playerHighscorePos].top, grid[10][4 + playerHighscorePos].right, grid[10][4 + playerHighscorePos].bottom - 3 };
				if (tick > 3)
					FillRect(hdc, &rect, borderBrush);
			};

			for (int i = 0; i <= 9; i++)
			{
				rect = grid[2][4 + i];
				sprintf(debugString, "#");
				strcat(debugString, std::to_string(i + 1).c_str());
				strcat(debugString, ": ");
				textheight = DrawTextA(hdc, debugString, -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, debugString, -1, &rect, DT_LEFT);

				rect = grid[4][4 + i];
				textheight = DrawTextA(hdc, sUser[i].c_str(), -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, sUser[i].c_str(), -1, &rect, DT_LEFT);

				rect = grid[7][4 + i];
				textheight = DrawTextA(hdc, std::to_string(iScore[i]).c_str(), -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, std::to_string(iScore[i]).c_str(), -1, &rect, DT_RIGHT);
			};

			if (tetrisGameover)
			{
				SelectObject(hdc, big_hFont);
				rect = { grid[1][17].left + 5, grid[1][17].top + 5, grid[10][17].right, grid[10][17].bottom + 5 };
				textheight = DrawTextA(hdc, "GAME OVER", -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, "GAME OVER", -1, &rect, DT_CENTER);
				SelectObject(hdc, s_hFont);
			};

			if (paused)
			{
				SelectObject(hdc, big_hFont);
				rect = { grid[3][17].left, grid[3][17].top + 5, grid[10][17].right, grid[10][17].bottom + 5 };
				textheight = DrawTextA(hdc, "PAUSED", -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, "PAUSED", -1, &rect, DT_CENTER);
				SelectObject(hdc, s_hFont);
			};

			if (bPlayerHighscore && bInputInitials)
			{
				rect = { grid[2][14].left + 5, grid[2][14].top + 12, grid[10][15].right, grid[10][15].bottom };//grid[1][9];
				textheight = DrawTextA(hdc, "Congratulations!\nPlease enter your initals:", -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, "Congratulations!\nPlease enter your initials:", -1, &rect, DT_CENTER);

				rect = { grid[5][16].left, grid[5][16].top + 3, grid[5][16].right, grid[5][16].bottom + 3 };
				textheight = DrawTextA(hdc, userInitials.c_str(), -1, &rect, DT_CALCRECT);
				rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
				DrawText(hdc, userInitials.c_str(), -1, &rect, DT_CENTER);
			};
		};

		DeleteObject(scoreboard);
		DeleteObject(lineboard);
		DeleteObject(levelboard);
		DeleteObject(whiteBrush);
		DeleteObject(debugString);
		DeleteObject(s_hFont);
		DeleteObject(big_hFont);
		DeleteObject(oneBrush);
		DeleteObject(twoBrush);
		DeleteObject(threeBrush);
		DeleteObject(fourBrush);
		DeleteObject(fiveBrush);
		DeleteObject(sixBrush);
		DeleteObject(sevenBrush);
		DeleteObject(backBrush);
		DeleteObject(borderBrush);
		DeleteObject(lineBrushA);
		DeleteObject(lineBrushB);
	};
};

tetrisGrid myGrid;

bool Game_Init()
{
	myGrid.init(19, 20, 16, 100, 22, 2); //At least this isn't taken by RAM before tetrisEnable.
	srand(time(NULL));
	return true;
};

void Game_Run(HWND window)
{
	if (!myGrid.paused && !myGrid.tetrisGameover)
	{
		myGrid.IncrementBlock();

		if (myGrid.blockStopped && (myGrid.linesToBeCleared == false) && (myGrid.clearLines == false))
		{
			myGrid.SpawnBlock(false, 0);
		};
	}
	else if (myGrid.bInputInitials)
	{
		myGrid.tick++; //keep counting the ticks for flashing on highscore screen.
		if (myGrid.tick >= 20)
		{
			myGrid.tick = 0;
		}
	}
	else
	{
		Sleep(100);
	};

	if (myGrid.failedWrite)
	{
		myGrid.CreateHighscores();
	};
	myGrid.ProjectBlock(); //Projects where the block will land if you press "spacebar"
	InvalidateRect(window, &windowRect, false); //Tells WM_PAINT to paint the screen.
};

void Game_End()
{
	MessageBox(0, "Game ended", "", 0);
};