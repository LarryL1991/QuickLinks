#pragma once

#include "resource.h"
#include "game.cpp"
#include <fstream>
#include <string>
void Game_Run(HWND window);
void Game_End();
inline bool ExistTest(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
};