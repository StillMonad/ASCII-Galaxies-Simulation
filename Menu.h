#pragma once
#include "ASCIIDrawer.h"
#include "LettersASCII.h"
#include "NumbersASCII.h"
#include "Res.h"
#include <vector>

class MenuPage
{
public:
	
};

class Menu
{
public:
	int currMenuPos = 0;
	int currPage = 0;
	int width = 0;
	bool thisPageToShow = true;
	std::vector<MenuPage*> pages;
	void act() {}
	void show(ASCIIDrawer d) {
		for (auto p : pages) {
			if (p->)
		}
	}
	void processInput(bool d, bool u, bool s) {
		if (d) --currMenuPos;
		if (u) ++currMenuPos;
		if (s) act();
		if (currMenuPos > len - 1) currMenuPos = 0;
		if (currMenuPos < 0) currMenuPos = len - 1;
	}
};

class MenuPage
{
public:

};

/*
d.clear();
currLen = 3;
d.drString("PAUSE", width / 2 - 100, 50, 5, 1);
d.drSprite(menuPos, width / 2 - 100, 100, 20, 5);
d.drString("SETTINGS", width / 2 - 75, 115, 2, 1);
d.drSprite(menuPos, width / 2 - 100, 130, 20, 5);
d.drString("RESUME", width / 2 - 75, 145, 2, 1);
d.drSprite(menuPos, width / 2 - 100, 160, 20, 5);
d.drString("EXIT", width / 2 - 75, 175, 2, 1);
d.drSprite(menuPosSel, width / 2 - 100, 100 + 30 * currMenuPos, 20, 5);
*/