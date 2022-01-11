#pragma once
#include <string>
#include <malloc.h>
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include "vec2.h"
#include "NumbersASCII.h"
#include "LettersASCII.h"
#include <iostream>

#undef max
#undef min

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

class ASCIIDrawer
{
public:
	float xMod = 0.9;
	float fisheye = 1.5;
	float treshold = 0.0;

	ASCIIDrawer(const int& w, const int& h, const float& xMod, const float& treshold = 0, const float& fisheye = 1.5) {
		this->w = w;
		this->h = h;
		this->xMod = xMod;
		this->treshold = treshold;
		this->fisheye = fisheye;
		this->screen    = new wchar_t[this->w * this->h];//(wchar_t*)malloc(this->w * this->h * sizeof(wchar_t));
		this->screenNum = new float[this->w * this->h]; //(float*)malloc(this->w * this->h * sizeof(float));
		this->zBuff     = new float[this->w * this->h]; //(float*)malloc(this->w * this->h * sizeof(float));
		this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
		//CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		//SetConsoleActiveScreenBuffer(hConsole); //{ (SHORT)150, (SHORT)30 }
		//CONSOLE_SCREEN_BUFFER_INFOEX consolesize;
		//GetConsoleScreenBufferInfoEx(hConsole, &consolesize);
		//SetConsoleScreenBufferInfoEx(hConsole, &consolesize);
		this->dwBytesWritten = 0;
		this->maxLen = this->w * this->h;
		clear();
	}

	~ASCIIDrawer() {
		delete[] screen;
		delete[] screenNum;
		delete[] zBuff;
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

	void customClear(const float& strength)
	{
		for (int i = 0; i < this->w * this->h; i++) {
			if (screenNum[i] == 0.0f) continue;
			screenNum[i] -= strength;
			screenNum[i] = clip(screenNum[i], 0.0f, 1.0f);
		}
	}

	int convertCoords(const int& x, const int& y) {   // origin is in the top-left corner
		return clip(y * this->w + x, 0, this->w * this->h - 1);
	}

	int convertCoordsGL(const float& x, const float& y) { // origin is in the center
		if (x < -this->w / 2 || x >= this->w / 2) return 0;
		if (y < -this->h / 2 || y >= this->h / 2) return 0;
		return (round(y) + this->h / 2 - 1) * this->w + x + this->w / 2 - 1;
		//return (round(clip(int(y), -h / 2, h / 2)) + this->h / 2 - 1) * this->w + clip(int(x), -w / 2, w / 2) + this->w / 2 - 1;
	}

	/*int convertCoordsGL(const float& x, const float& y) { // origin is in the center
		vec2 ret = { x/xMod, y*2 };
		//vec2 ret = { x, y };
		float dist = pow(0.001*(ret.magnitude()), 0.9) * 1000;
		ret.normalize();
		ret = ret * dist;
		ret.x *= xMod;
		ret.y /= 2;
		if (ret.x < -this->w / 2 || ret.x >= this->w / 2) return 0;
		if (ret.y < -this->h / 2 || ret.y >= this->h / 2) return 0;
		return (round(ret.y) + this->h / 2 - 1) * this->w + ret.x + this->w / 2 - 1;
	}*/

	void drNumber(const float& num, const int& px, const int& py, const float& size, const float& color, const int& len = 1) {
		std::string numS;
		if (num == (int)num)
			numS = std::to_string((int)num);
		else 
			numS = std::to_string(num);
		int ind = 0;
		int pPos = 0;
		for (int i = 0; i < numS.size(); ++i) {
			int oneN = (int)numS[i] - 48;
			if (pPos > len)  return;
			if (numS[i] == '.') oneN = 10;
			if (numS[i] == '-') oneN = 11;
			if (numS[i] == 'n') {
				drString("NAN", px + 7 * size * ind, py, size, color);
				return;
			}
			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (numbers[oneN][(int)(y / size) * 6 + (int)(x / size)] != ' ') {
						screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
						this->zBuff[convertCoords(x + px + ind * 7 * size, y + py)] = -10;
					}
				}
			}
			pPos = pPos > 0 ? ++pPos : 0;
			++ind;
		}
	}

	void drString(std::string str, const int& px, const int& py, const float& size, const float& color) {
		int ind = 0;
		for (auto& c : str) c = toupper(c);
		for (int i = 0; i < str.size(); ++i) {
			if (str[i] == ' ') {
				++ind;
				continue;
			}
			int oneL = (int)str[i] - 65;
			if (str[i] == ',') oneL = 26;
			if (str[i] == '.') oneL = 27;
			if (str[i] == ':') oneL = 28;
			if (str[i] == '"') oneL = 29;
			if (str[i] == '-') oneL = 30;
			if (str[i] == '(') oneL = 31;
			if (str[i] == ')') oneL = 32;
			if (str[i] == '=') oneL = 33;
			if (str[i] == '*') oneL = 34;
			if (str[i] == '/') oneL = 35;
			if (0 <= str[i] - 48 && str[i] - 48 < 10) {
				drNumber(int(str[i]) - 48, px + ind * 7 * size, py, 1, 1, 0);
				++ind;
				continue;
			}

			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (letters[oneL][(int)(y / size) * 6 + (int)(x / size)] != ' ') {
						screenNum[convertCoords(clip(x + px + ind * 7 * size, 0.0f, (float)w - 1.0f), clip(y + (float)py, 0.0f, (float)h - 1.0f))] = color;
						this->zBuff[convertCoords(clip(x + px + ind * 7 * size, 0.0f, (float)w - 1.0f), clip(y + (float)py, 0.0f, (float)h - 1.0f))] = -10;
					}
				}
			}
			++ind;
		}
	}

	void drSprite(const std::vector <std::vector <float>>& vec, const int& px, const int& py, const float& sizeX, const float& sizeY) {
		int ind = 0;
			for (int x = 0; x < vec[0].size() * sizeX; ++x) {
				for (int y = 0; y < vec.size() * sizeY; ++y) {
					screenNum[convertCoords(x + px, y + py)] += vec[(int)((float)y / sizeY)][(int)((float)x / sizeX)];
				}
			}
			++ind;
	}

	void drPointOpaqueNum(const float& px, const float& py, const float& color) {
		int pos = convertCoordsGL(px, py);
		//if (px < -this->w / 2 || px >= this->w / 2) return;
		//if (py < -this->h / 2 || py >= this->h / 2) return;
		if (pos < 0 || pos >= maxLen) return;
		//int i = 0;
		//this->screenNum[pos] = clip(this->screenNum[pos] + color, 0.0f, 1.0f);
		this->screenNum[pos] = this->screenNum[pos] + color;
	}

	void drPointNum(const float& px, const float& py, const float& color) {
		int pos = convertCoordsGL(px, py);
		//if (px < -this->w / 2 || px >= this->w / 2) return;
		//if (py < -this->h / 2 || py >= this->h / 2) return;
		if (pos < 0 || pos >= this->h * this->w) return;
		int i = 0;
		//this->screenNum[pos] = clip(this->screenNum[pos] + color, 0.0f, 1.0f);
		this->screenNum[pos] = color;
	}

	bool smartCmp(const float& a, const float& b, const float& c) {
		if (b <= c) return a <= c;
		else return a > c;
	}

	void drLineOpaqueNum(const float& sx, const float& sy, const float& ex, const float& ey, const float& color, const int& style, bool opaque = true) {
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
		int maxLen = w;
		while ((smartCmp(i, sx, ex) || smartCmp(j, sy, ey)) && maxLen > 0) {
			//if (p % style == 0) 
			if (opaque)
				drPointOpaqueNum(i, j, color);
			else
				drPointNum(i, j, color);
			i += dX / len * style;
			j += dY / len * style;
			p += style;
			--maxLen;
		}
	}

	void drCross(const float& px, const float& py, const float& size, const float& color, const int& style) {
		drLineOpaqueNum((px - size * xMod), py - size/2.0, (px + size * xMod), py + size/2.0, color, style);
		drLineOpaqueNum((px - size * xMod), py + size/2.0, (px + size * xMod), py - size/2.0, color, style);
	}

	void drArrow(const float& px, const float& py, vec2 dir, const float& color, const int& style) {
		vec2 dc = -dir;
		//dc.x = dc.x * xMod;
		dc.normalize();
		float ax = px + dir.x * xMod;
		float ay = py + dir.y / 2.0;
		float ang = M_PI / 8;
		vec2 d1 = dc.rotate( ang) * 10;
		vec2 d2 = dc.rotate(-ang) * 10;
		drLineOpaqueNum(px, py, ax, ay, color, style);
		drLineOpaqueNum(ax, ay, ax + d1.x, ay + d1.y / 2, color, 1);
		drLineOpaqueNum(ax, ay, ax + d2.x, ay + d2.y / 2, color, 1);
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

	int getColor(const float& col) {
		if (col <= treshold) return 0;
		if (col >= 1) return color_scheme.size() - 1;
		//return (int)clip((((float)color_scheme.size() - 1) * col + (rand() % 2000 / 4000.0)), 0.0, (double)color_scheme.size() - 1);
		//return (int)clip((((float)color_scheme.size() - 1) * col + (rand() % 4000 / 250.0 - 4) * oneSym), 0.0, (double)color_scheme.size() - 1);
		//return (int)clip(((color_scheme.size() - 1) * (double)col), 0.0, (double)color_scheme.size() - 1);
		int (*getS)() = []() { int s = rand() % 2; return s == 1 ? 1 : -1; };
		return (int)clip(round(((float)color_scheme.size() - 1) * col + getS() * (13.0 / double((rand() % 1000) / 100.0 + 1.0)) * 0.05), 0.0, (double)color_scheme.size() - 1);
	}

	int showNum() {
		typedef std::chrono::high_resolution_clock Clock;
		auto start = Clock::now();
		for (int i = 0; i < this->w * this->h; ++i) { 
			screen[i] = color_scheme[std::max(getColor(screenNum[i]), 0)];
		}
		//WriteConsoleOutputCharacter(hConsole, screen, this->w * this->h, { 0,0 }, &dwBytesWritten);
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), screen, this->w * this->h, &dwBytesWritten, NULL);
		auto end = Clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}

	void rebright(float a) {
		for (int i = 0; i < this->w * this->h; ++i) {
			screenNum[i] = pow(screenNum[i], a);
		}
	}

	std::string getColorScheme() { return color_scheme; }

	void checkPalette() {
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
		for (int i = 0; i < this->w; ++i) {
			for (int j = 0; j < this->h; ++j) {
				this->screenNum[convertCoords(i, j)] = ((float)i / ((float)this->w - this->w*oneSym));
			}
		}
	}

	void setConsoleSize(int width, int height) {
		_COORD coord;
		coord.X = width;
		coord.Y = height;
		_SMALL_RECT Rect;
		Rect.Top = 0;
		Rect.Left = 0;
		Rect.Bottom = height - 1;
		Rect.Right = width - 1;
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleScreenBufferSize(h, coord);
		SetConsoleWindowInfo(h, TRUE, &Rect);
	}

	void setConsoleFont(float width, float height) {
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = width;
		cfi.dwFontSize.Y = height;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = 500;
		wcscpy_s(cfi.FaceName, L"Lucida Console"); //Courier New, Lucida Console, Consolas
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
	}

private:

	 //=================================//
	// Here u can choose color schemes //
   //=================================//
	//std::string color_scheme = " .,:-~=<+xvzXY#&8%B@$";
	//std::string color_scheme = ".=#*+%";  // --- good for 3pt
	//std::string color_scheme = " `*xhG=&";  // --- good for 1pt
	std::string color_scheme = " `*hG=&";  // --- good for 1pt // FW500
	//std::string color_scheme = " `*xshFG=&";  // --- good for 1pt
	//std::string color_scheme = " `.;!=*&#W";  // -- good for 2pt
	//std::string color_scheme = ".,:irs?9B&#@$";
	//std::string color_scheme = " .,ilwW";  // --- good for 3pt
	//std::string color_scheme = " -.^?HRNMW";  // -- good for 2pt  Courier New  -- best
	//std::string color_scheme = " .,'^HRW";  // -- good for 2x2pt  Courier New  -- best
	//std::string color_scheme = " -`.;^?*RHNW";  // -- good for 2pt  Courier New  -- best
	//std::string color_scheme = " .;iwW";  //
	//std::string color_scheme = " .+^x%w@8";  //  -- 600x200 3pt Lucida console
	//std::string color_scheme = "'.\'^,:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
	//std::string color_scheme = "1234567890-=`!@#$%^&*()_+~¹;%:?*qwertyuiop[]\\asdfghjkl;'zxcvbnm,./QWERTYUIOP{}|ASDFGHJKL:ZXCVBNM<>?";
	//std::string color_scheme = ".,:irs?9B&@";  
	//std::string color_scheme = " '*|tcYCFgqd%"; // blueish-greenish tone on 1pt  //T40 - reddish symbols
	//std::string color_scheme = "3{O"; // yellowish
	//std::string color_scheme = "5?NVZ79-5@?i[]fR2"; //reddish tone on 1pt
	//std::string color_scheme = "_;`!T16=$^&*()~¹}ASGHJKL:XB<>:*eryu\\hjkl;zxvnm,.QUIP"; //grayish
	//std::string color_scheme = "sopb"; //greenish
	//std::string color_scheme = "`/+E0Ww4#aM"; //purplish
	//std::string color_scheme = "`'\":?s}#%W@"; // --- best palette for 5pt 
	//std::string color_scheme = " '*YC4#6=&"; //mixed colors

	int w, h;
	int maxLen = w * h;
	wchar_t *screen;
	float* screenNum;
	float* zBuff;
	DWORD dwBytesWritten = 0;
	HANDLE hConsole;
	float prec = 0.8;
	float oneSym = 1.0 / this->color_scheme.size();
	POINT p;
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
};