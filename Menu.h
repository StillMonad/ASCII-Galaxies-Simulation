#pragma once
#define _USE_MATH_DEFINES
#include "ASCIIDrawer.h"
//#include "LettersASCII.h"
//#include "NumbersASCII.h"
//#include "Res.h"
#include <vector>


class MenuPage
{
public:
	int length = 0;
	std::vector<int> ref;
	MenuPage(int len, std::vector<int>& r)
	{
		length = len;
		ref = r;
	}
	int act(int pos) {
		return ref[pos];
	}

	virtual void show(int cPos) {}
};

class Menu
{
public:
	int currMenuPos = 0;
	int currPage = 0;
	int w = 0;
	int h = 0;
	std::vector<MenuPage> pages = {};
	Menu(std::vector<MenuPage> pages, int width, int height) {
		this->pages = pages;
		this->w = width;
		this->h = height;
	}

	void show() {
		Drawer.drString("PAUSE", width / 2 - 100, 50, 5, 1);
		//MenuPage p;
		pages[currPage].show(currMenuPos);
	}
	void processInput(bool d, bool u, bool s) {
		int len = pages[currPage].length;
		if (d) --currMenuPos;
		if (u) ++currMenuPos;
		if (s) {
			currMenuPos = 0;
			currPage = pages[currPage].act(currMenuPos);
		}
		if (currMenuPos > len - 1) currMenuPos = 0;
		if (currMenuPos < 0) currMenuPos = len - 1;
	}
};


class MenuPage1 : public MenuPage
{
public:
	MenuPage1(int len, std::vector<int>& r) : MenuPage(len, r) {}
	void show(int cPos) {
		Drawer.clear();
		Drawer.drString("PAUSE", width / 2 - 100, 50, 5, 1);
		Drawer.drSprite(SPRITE_menuPos, width / 2 - 100, 100, 20, 5);
		Drawer.drString("SETTINGS", width / 2 - 75, 115, 2, 1);
		Drawer.drSprite(SPRITE_menuPos, width / 2 - 100, 130, 20, 5);
		Drawer.drString("RESUME", width / 2 - 75, 145, 2, 1);
		Drawer.drSprite(SPRITE_menuPos, width / 2 - 100, 160, 20, 5);
		Drawer.drString("EXIT", width / 2 - 75, 175, 2, 1);
		Drawer.drSprite(SPRITE_menuPosSel, width / 2 - 100, 100 + 30 * cPos, 20, 5);
	}
};
