#pragma once
#include <string>
#include <malloc.h>
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include "vec2.h"
#include "NumbersASCII.h"
#include "LettersASCII.h"

#undef max
#undef min

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

class ASCIIDrawer
{
public:
	ASCIIDrawer(int w, int h) {
		this->w = w;
		this->h = h;
		this->screen = (wchar_t*)malloc(this->w * this->h * sizeof(wchar_t));
		this->screenNum = (float*)malloc(this->w * this->h * sizeof(float));
		this->zBuff = (float*)malloc(this->w * this->h * sizeof(float));
		this->hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole);
		this->dwBytesWritten = 0;
		clear();
	}
	
	/*vec2 getCursorPos() {
		GetConsoleScreenBufferInfo(this->hConsole, &cbsi);
		GetCursorPos(&p);
		vec2 r;

		if (ScreenToClient(GetConsoleWindow(), &p))
		{
			r = { ((float)p.x)/2.4f, ((float)p.y)/2.4f };
		}
		
		return r;
	}*/

	void clear()
	{
		for (int i = 0; i < this->w * this->h; i++) {
			screen[i] = ' ';
			zBuff[i] = 0;
			screenNum[i] = 0.0f;
		}
	}

	void customClear(float str)
	{
		for (int i = 0; i < this->w * this->h; i++) {
			if (screenNum[i] == 0.0f) continue;
			screenNum[i] = clip(screenNum[i] - str, 0.0f, 1.0f);
		}
	}

	int convertCoords(int x, int y) {   // origin is in the top-left corner
		return y * this->w + x;
	}

	int convertCoordsGL(float x, float y) { // origin is in the center
		return (round(y) + this->h / 2 - 1) * this->w + x + this->w / 2 - 1;
	}

	void drNumber(float num, int px, int py, float size, float color) {
		std::string numS = std::to_string(num);
		int ind = 0;
		for (int i = 0; i < numS.size(); ++i) {
			int oneN = (int)numS[i] - 48;
			if (numS[i] == '.') {
				oneN = 10;
				for (int x = 0; x < 6 * size; ++x) {
					for (int y = 0; y < 5 * size; ++y) {
						if (numbers[oneN][(int)(y / size) * 6 + (int)(x / size)] != ' ') screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
					}
				}
				++ind;
				continue;
			}
			if (numS[i] == '-') {
				oneN = 11;
				for (int x = 0; x < 6 * size; ++x) {
					for (int y = 0; y < 5 * size; ++y) {
						if (numbers[oneN][(int)(y / size) * 6 + (int)(x / size)] != ' ') screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
					}
				}
				++ind;
				continue;
			}
			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (numbers[oneN][(int)(y / size) * 6 + (int)(x / size)] != ' ') screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
				}
			}
			++ind;
		}
	}

	void drString(std::string str, int px, int py, float size, float color) {
		int ind = 0;
		for (int i = 0; i < str.size(); ++i) {
			if (str[i] == ' ') {
				++ind;
				continue;
			}
			int oneL = (int)str[i] - 65;
			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (letters[oneL][(int)(y/size) * 6 + (int)(x/size)] != ' ') screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
				}
			}
			++ind;
		}
	}

	void drSprite(std::vector <std::vector <float>> vec, int px, int py, float sizeX, float sizeY) {
		int ind = 0;
			for (int x = 0; x < vec[0].size() * sizeX; ++x) {
				for (int y = 0; y < vec.size() * sizeY; ++y) {
					screenNum[convertCoords(x + px, y + py)] += vec[(int)((float)y / sizeY)][(int)((float)x / sizeX)];
				}
			}
			++ind;
	}

	void drPoint(int px, int py, float color, float depth) {
		int pos = convertCoords(px, py);
		if (px < 0 || px >= this->w) return;
		if (py < 0 || py >= this->h) return;
		this->screen[pos] = color_scheme[clip((int)(color * (color_scheme.size() - 1)), 0, (int)color_scheme.size() - 1)];
		this->zBuff[pos] = depth;
	}

	void drPointOpaque(float px, float py, float color, float depth) {
		int pos = convertCoordsGL(px, py);
		if (px < -this->w / 2 || px >= this->w / 2) return;
		if (py < -this->h / 2 || py >= this->h / 2) return;
		if (pos < 0 || pos >= this->h * this->w) return;
		int i = 0;
		if ((char)screen[pos] != ' ')
			for (i = 0; i < color_scheme.size(); i++) {
				if ((char)screen[pos] == (color_scheme[i])) break;
			}
		//i = 0;
		this->screen[pos] = color_scheme[clip((int)(i + (int)round(color * (float)(color_scheme.size() - 1))), 0, (int)color_scheme.size() - 1)];
		this->zBuff[pos] = depth;
	}

	void drPointOpaqueNum(float px, float py, float color) {
		int pos = convertCoordsGL(px, py);
		if (px < -this->w / 2 || px >= this->w / 2) return;
		if (py < -this->h / 2 || py >= this->h / 2) return;
		if (pos < 0 || pos >= this->h * this->w) return;
		int i = 0;
		//this->screenNum[pos] = clip(this->screenNum[pos] + color, 0.0f, 1.0f);
		this->screenNum[pos] = this->screenNum[pos] + color;
	}

	bool smartCmp(float a, float b, float c) {
		if (b <= c) return a <= c;
		else return a > c;
	}

	void drLineOpaqueNum(float sx, float sy, float ex, float ey, float color, int style) {
		float dX = ex - sx;
		dX = dX == 0 ? 0.1 : dX;
		float dY = ey - sy;
		dY = dY == 0 ? 0.1 : dY;
		float len = sqrt(dX * dX + dY * dY);
		int sideX = (ex - sx) > 0 ? 1 : -1;
		int sideY = (ey - sy) > 0 ? 1 : -1;
		float i = sx;
		float j = sy;
		int p = 0;
		//for (float i = sx, float j = sy; smartCmp(i, sx, ex); i += dX / len, j += dY / len) {
		while (smartCmp(i, sx, ex) || smartCmp(j, sy, ey)) {
			if (p % style == 0) drPointOpaqueNum(i, j, color);
			i += dX / len;
			j += dY / len;
			++p;
		}
	}

	void drCross(float px, float py, float size, float color) {
		//drLineOpaqueNum(px - size, py, px + size, py, color);
		//drLineOpaqueNum(px, py - size/3.0, px, py + size/3.0, color);
		drLineOpaqueNum(px - size * 0.7, py - size/2.0, px + size * 0.7, py + size/2.0, color, 1);
		drLineOpaqueNum(px - size * 0.7, py + size/2.0, px + size * 0.7, py - size/2.0, color, 1);
	}

	void drArrow(float px, float py, vec2 dir, float color) {
		drLineOpaqueNum(px, py, px + dir.x * 0.7, py + dir.y / 2.0, color, 2);
		vec2 dc = - dir;
		dc.normalize();
		float ax = px + dir.x * 0.7;
		float ay = py + dir.y / 2.0;
		float ang1 = 5 * M_PI / 6;
		float ang2 = 1 * M_PI / 6;
		dc = dc * 5;
		float angX = dc.getAngToX();
		drLineOpaqueNum(ax, ay, ax - cos(ang1 - angX) * dc.magnitude(), ay - sin(ang1 - angX) * dc.magnitude(), color, 1);
		drLineOpaqueNum(ax, ay, ax + cos(ang2 - angX) * dc.magnitude(), ay + sin(ang2 - angX) * dc.magnitude(), color, 1);
		//drLineOpaqueNum(ax, ay, ax + dc.x * cos(-ang) * 0.7, ay + dc.y * sin(-ang) / 2, color, 1);
	}

	void drCircle(int px, int py, int rad, float color, float depth, float fill) {
		float inc = 1 / ((float)this->prec * (float)rad);
		float c = 0;
		if (px < 0 || px > this->w)
			if (px + rad < 0 || px - rad > this->w)
				return;
		if (py < 0 || py > this->w)
			if (py + rad < 0 || py - rad > this->w)
				return;
		for (int i = px - rad; i < px + rad; ++i) {
			for (int j = py - rad; j < py + rad; ++j) {
				if (pow((i - px),2) + pow((j - py),2) < rad * rad) drPoint(i, j/2, fill, depth);
			}
		}
		for (float c = 0; c < 2 * 3.14; c += inc) {
			drPoint(round(sin(c) * (float)rad) + px, (round(cos(c) * (float)rad) / 2.0 + py / 2.0), color, depth);
		}
	}

	void drCircleOpaqueNum(int px, int py, int rad, float color, float depth, float fill) {
		float inc = 1 / ((float)this->prec * (float)rad);
		float c = 0;
		int pos = convertCoordsGL(px, py);
		if (px < -w/2 || px > w/2)
			if (px + rad < -w/2 || px - rad > w/2)
				return;
		if (py < -h/2 || py > h/2)
			if (py + rad < -h/2 || py - rad > h/2)
				return;
		for (int i = px - rad; i < px + rad; ++i) {
			for (int j = py - rad; j < py + rad; ++j) {
				if (pow((i - px), 2) + pow((j - py), 2) < rad * rad) drPointOpaqueNum(i, j / 3, fill);
			}
		}
		for (float c = 0; c < 2 * 3.14; c += inc) {
			drPointOpaqueNum(round(sin(c) * (float)rad) + px, (round(cos(c) * (float)rad) / 3.0 + py / 3.0), color);
		}
	}

	void show() {
		for (int i = 0; i < this->w; i++) {
			this->screen[i] = this->color_scheme[this->color_scheme.size() - 1];
			this->screen[(this->h - 1) * this->w + i] = this->color_scheme[this->color_scheme.size() - 1];
		}

		for (int i = 0; i < this->h; i++) {
			int pos = convertCoords(this->w - 1, i);
			this->screen[pos] = '\n';
			this->screen[pos-4] = this->color_scheme[this->color_scheme.size() - 1];
			this->screen[i * w] = this->color_scheme[this->color_scheme.size() - 1];
		}
		WriteConsoleOutputCharacter(hConsole, screen, this->w * this->h, { 0,0 }, &dwBytesWritten);
	}

	int getColor(float col) {
		//return (int)(((float)color_scheme.size() - 1) * col + (rand() % 1000 / 125.0  - 4) * oneSym);
		//return (int)(((float)color_scheme.size() - 1.0) * col);
		if (col == 0) return 0;
		if (col == 1) return color_scheme.size() - 1;
		return (int)clip((((float)color_scheme.size() - 1) * col + (rand() % 4000 / 125.0 - 8) * oneSym), 0.0, (double)color_scheme.size() - 1);
	}

	void showNum() {
		for (int i = 0; i < this->w * this->h; ++i) { 
			//if (screenNum[i] == 0) screen[i] = ' ';
			//else screen[i] = color_scheme[std::max(getColor(screenNum[i]), 0)]; 
			screen[i] = color_scheme[std::max(getColor(screenNum[i]), 0)];
		}
		WriteConsoleOutputCharacter(hConsole, screen, this->w * this->h, { 0,0 }, &dwBytesWritten);
	}

	void rebright(float a) {
		for (int i = 0; i < this->w * this->h; ++i) {
			screenNum[i] = pow(screenNum[i], a);
		}
	}

	std::string getColorScheme() { return color_scheme; }

	void checkPalette() {
		float oneSym = 1.0 / this->color_scheme.size();
		int linesC = 5;// (int)(oneSym * this->h);
		int sym = 0;
		for (int i = 0; i < color_scheme.size(); ++i) {
			for (int j = 0; j < linesC; ++j) {
				for (int k = 0; k < this->w; ++k) {
					this->screen[i * w * linesC + w * j + k] = this->color_scheme[i];
				}
			}
		}
	}

	void checkPaletteNum() {
		float oneSym = 1.0 / this->color_scheme.size();
		int sym = 0;
		for (int i = 0; i < this->w; ++i) {
			for (int j = 0; j < this->h; ++j) {
				this->screenNum[convertCoords(i, j)] = ((float)i / (float)this->w);
			}
		}
	}

private:
	int w, h;
	//std::string color_scheme = " .,:-~=<+xvzXY#&8%B@$";
	//std::string color_scheme = ".=*+#%";  // --- good for 3pt
	std::string color_scheme = " `.;!=*&#W";  
	//std::string color_scheme = ".,:irs?9B&#@$";
	//std::string color_scheme = " .,ilwW";  // --- good for 3pt
	//std::string color_scheme = ".;iwW";  //
	//std::string color_scheme = "'.\'^,:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
	//std::string color_scheme = ".,:irs?9B&@";  
	//std::string color_scheme = "`'\":?s}#%W@"; // --- best palette for 5pt 
	float oneSym = 1.0 / (float)color_scheme.size();
	wchar_t* screen;
	float* screenNum;
	float* zBuff;
	DWORD dwBytesWritten = 0;
	HANDLE hConsole;
	float prec = 0.8;
	POINT p;
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
};