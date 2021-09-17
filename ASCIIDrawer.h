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

#undef max
#undef min

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

class ASCIIDrawer
{
public:
	float xMod = 0.7;

	ASCIIDrawer(const int& w, const int& h, const float& xMod) {
		this->w = w;
		this->h = h;
		this->xMod = xMod;
		this->screen = (wchar_t*)malloc(this->w * this->h * sizeof(wchar_t));
		this->screen1 = (wchar_t*)malloc(this->w * this->h / 2 * sizeof(wchar_t));
		this->screen2 = (wchar_t*)malloc(this->w * this->h / 2 * sizeof(wchar_t));
		this->screenNum = (float*)malloc(this->w * this->h * sizeof(float));
		this->zBuff = (float*)malloc(this->w * this->h * sizeof(float));
		this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
		//CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole); //{ (SHORT)150, (SHORT)30 }
		//CONSOLE_SCREEN_BUFFER_INFOEX consolesize;
		//GetConsoleScreenBufferInfoEx(hConsole, &consolesize);
		//SetConsoleScreenBufferInfoEx(hConsole, &consolesize);
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

	void customClear(const float& strength)
	{
		for (int i = 0; i < this->w * this->h; i++) {
			if (screenNum[i] == 0.0f) continue;
			screenNum[i] = clip(screenNum[i] - strength, 0.0f, 1.0f);
		}
	}

	int convertCoords(const int& x, const int& y) {   // origin is in the top-left corner
		return clip(y * this->w + x, 0, this->w * this->h - 1);
	}

	int convertCoordsGL(const float& x, const float& y) { // origin is in the center
		return (round(y) + this->h / 2 - 1) * this->w + x + this->w / 2 - 1;
	}

	void drNumber(const float& num, const int& px, const int& py, const float& size, const float& color, const int& len = 0) {
		std::string numS = std::to_string(num);
		int ind = 0;
		int pPos = 0;
		for (int i = 0; i < numS.size(); ++i) {
			int oneN = (int)numS[i] - 48;
			if (pPos > len)  return;
			if (numS[i] == '.') {
				pPos += 1;
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
			if (numS[i] == 'n') {
				drString("NAN", px + 7 * size * ind, py, size, color);
				return;
			}
			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (numbers[oneN][(int)(y / size) * 6 + (int)(x / size)] != ' ') 
						screenNum[convertCoords(x + px + ind * 7 * size, y + py)] = color;
				}
			}
			pPos = pPos > 0 ? ++pPos : 0;
			++ind;
		}
	}

	void drString(const std::string& str, const int& px, const int& py, const float& size, const float& color) {
		int ind = 0;
		for (int i = 0; i < str.size(); ++i) {
			if (str[i] == ' ') {
				++ind;
				continue;
			}
			int oneL = (int)str[i] - 65;
			for (int x = 0; x < 6 * size; ++x) {
				for (int y = 0; y < 5 * size; ++y) {
					if (letters[oneL][(int)(y/size) * 6 + (int)(x/size)] != ' ') 
						screenNum[convertCoords(clip(x + px + ind * 7 * size, 0.0f, (float)w-1.0f), clip(y + (float)py, 0.0f, (float)h-1.0f))] = color;
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

	void drPoint(const int& px, const int& py, const float& color, const float& depth) {
		int pos = convertCoords(px, py);
		if (px < 0 || px >= this->w) return;
		if (py < 0 || py >= this->h) return;
		this->screen[pos] = color_scheme[clip((int)(color * (color_scheme.size() - 1)), 0, (int)color_scheme.size() - 1)];
		this->zBuff[pos] = depth;
	}

	void drPointOpaque(const float& px, const float& py, const float& color, const float& depth) {
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

	void drPointOpaqueNum(const float& px, const float& py, const float& color) {
		int pos = convertCoordsGL(px, py);
		if (px < -this->w / 2 || px >= this->w / 2) return;
		if (py < -this->h / 2 || py >= this->h / 2) return;
		if (pos < 0 || pos >= this->h * this->w) return;
		int i = 0;
		//this->screenNum[pos] = clip(this->screenNum[pos] + color, 0.0f, 1.0f);
		this->screenNum[pos] = this->screenNum[pos] + color;
	}

	bool smartCmp(const float& a, const float& b, const float& c) {
		if (b <= c) return a <= c;
		else return a > c;
	}

	void drLineOpaqueNum(const float& sx, const float& sy, const float& ex, const float& ey, const float& color, const int& style) {
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
		while (smartCmp(i, sx, ex) || smartCmp(j, sy, ey)) {
			if (p % style == 0) drPointOpaqueNum(i, j, color);
			i += dX / len;
			j += dY / len;
			++p;
		}
	}

	void drCross(const float& px, const float& py, const float& size, const float& color, const int& style) {
		drLineOpaqueNum(px * xMod - size * xMod, py - size/2.0, px * xMod + size * xMod, py + size/2.0, color, style);
		drLineOpaqueNum(px * xMod - size * xMod, py + size/2.0, px * xMod + size * xMod, py - size/2.0, color, style);
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

	int getColor(const float& col) {
		if (col == 0) return 0;
		if (col == 1) return color_scheme.size() - 1;
		return (int)clip((((float)color_scheme.size() - 1) * col + (rand() % 1000 / 5000.0 - 0.1)), 0.0, (double)color_scheme.size() - 1);
		//return (int)clip((((float)color_scheme.size() - 1) * col + (rand() % 4000 / 250.0 - 4) * oneSym), 0.0, (double)color_scheme.size() - 1);
		//return (int)clip(((color_scheme.size() - 1) * (double)col), 0.0, (double)color_scheme.size() - 1);
	}

	void threadShow1() {
		WriteConsoleOutputCharacter(hConsole, screen1, this->w * this->h / 2, { 0,0 }, &dwBytesWritten);
	}
	void threadShow2() {
		WriteConsoleOutputCharacter(hConsole, screen2, this->w * this->h / 2, { 0, (SHORT)(this->h / 2) }, &dwBytesWritten);
	}
	std::thread THREADthreadShow1() {
		return std::thread([=] { threadShow1(); });
	}
	std::thread THREADthreadShow2() {
		return std::thread([=] { threadShow2(); });
	}

	int threadShowNum() {
		typedef std::chrono::high_resolution_clock Clock;
		auto start = Clock::now();
		for (int i = 0; i < this->w * this->h/2; ++i) {
			screen1[i] = color_scheme[std::max(getColor(screenNum[i]), 0)];
		}
		for (int i = this->w * this->h / 2, j = 0; i < this->w * this->h - 2; ++i) {
			screen2[j] = color_scheme[std::max(getColor(screenNum[i]), 0)];
			++j;
		}
		std::thread thr1 = THREADthreadShow1();
		std::thread thr2 = THREADthreadShow2();
		thr1.join();
		thr2.join();
		auto end = Clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}

	int showNum() {
		typedef std::chrono::high_resolution_clock Clock;
		auto start = Clock::now();
		for (int i = 0; i < this->w * this->h; ++i) { 
			screen[i] = color_scheme[std::max(getColor(screenNum[i]), 0)];
		}
		WriteConsoleOutputCharacter(hConsole, screen, this->w * this->h, { 0,0 }, &dwBytesWritten);
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

private:
	int w, h;
	 //=================================//
	// Here u can choose color schemes //
   //=================================//
	//std::string color_scheme = " .,:-~=<+xvzXY#&8%B@$";
	//std::string color_scheme = ".=#*+%";  // --- good for 3pt
	std::string color_scheme = " `*xhG=&";  // --- good for 1pt
	//std::string color_scheme = " `.;!=*&#W";  // -- good for 2pt
	//std::string color_scheme = ".,:irs?9B&#@$";
	//std::string color_scheme = " .,ilwW";  // --- good for 3pt
	//std::string color_scheme = ".;iwW";  //
	//std::string color_scheme = "'.\'^,:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
	//std::string color_scheme = ".,:irs?9B&@";  
	//std::string color_scheme = "`'\":?s}#%W@"; // --- best palette for 5pt 
	wchar_t *screen, *screen1, *screen2;
	float* screenNum;
	float* zBuff;
	DWORD dwBytesWritten = 0;
	HANDLE hConsole;
	float prec = 0.8;
	float oneSym = 1.0 / this->color_scheme.size();
	POINT p;
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
};