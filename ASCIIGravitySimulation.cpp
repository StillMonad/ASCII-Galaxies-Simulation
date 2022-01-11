
#define _USE_MATH_DEFINES
#include "ASCIIDrawer.h"
#include "vec2.h"
#include "Res.h"
#include "keyEvents.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <thread>
#include <csignal>
#include <mutex>

//==============================================================================================//
//==========================              GENERATION SEED              =========================//
//==============================================================================================//
//rand() generation seed 
		int SEED = 6;

//==============================================================================================//
//==========================               CONST SECTION               =========================//
//==========================       FEEL FREE TO CHANGE SOMETHING       =========================//
//==============================================================================================//
		// if true, draws color test instead
			bool colorTest = 0;
		// max frames per second 
			const int   fps    = 20;
		// console width
			const int   width  = 700;  
		// console height
			const int   height = 400;   
		// x modifier for making circles circular
			const float xMod   = 0.5f;   
		// minimum physics frames recalculations between drawing
			const int minRecFrames = 1; // not working now
		// gravity const G
			const float G = 1.0f;
		// gravity fading power (pow(R, Gpow)), regular is 2
			const float Gpow = 2.0f;
		// initial camera position
			float px = 0;
			float py = 0;
		// initial zoom
			float zoom = 50.0;
		// initial time speed
			float speed = 0.01;

// ======= every particle is drawn like: color = clip(pow(brightness, brPower), treshold, 1.0) =======
		
		// initial brightness (brightness is a color intensity multiplier) 
			float brightness = 0.003;
		// initial clear power (blur) - 1 clears eveverything, 0 clears nothing, something in between create motion blur
			float clPower = 0.8f;
		// initial brightness power (pow(brightness, brPower)) can shift colors more to bright or dark
			float brPower = 0.5f;
		// start drawing only if color is higher than treshold
			float treshold = 0.1;
		// fisheye
			float fisheye = 10.5; // not working now

//===========================    galaxies generation (randomized)     ==========================//
		// galaxies count
			int gCount = 3;     
		// stars count in one galaxy
			int gSizeMax = 250000;        
			int gSizeMin = 120000; 
		// top right bottom left corners of possible positions 
			vec2 gPosMax = {  5,  5 }; 
			vec2 gPosMin = { -5, -5 }; 
		// mass of one galaxy center
			int gMassMax = 1000;           
			int gMassMin = 100;     
		// velocity of galaxies
			int gVelMax  = 20;       
			int gVelMin  = -20;          
		// radius of a galaxy
			int gRadMax = 10;
			int gRadMin = 10;
		// velocities anomaly (velocity = ideal obital velocity + anomaly)
			float velAnom = 2.0;
  //==============================================================================================//
 //===================================   END OF CONST SECTION   =================================//
//==============================================================================================//

int getRandNum(const int& min, const int& max) {
	int diff = max - min;
	if (diff == 0) return 0;
	return (rand() % diff) + min;
}

int getRandSign() {
	int r = getRandNum(-1, 1);
	return r != 0 ? r : getRandSign();
}

inline vec2 projToScreen(float x, float y) {
	return { (x - px) * zoom * xMod, (y - py) * zoom / 2.0f };
}

struct ball {
	float mass;
	float rad;
	vec2 pos;
	vec2 vel;
	int fixed;
	vec2 acc;
};

struct galaxyCenter {
	vec2 pos;
	vec2 vel;
	vec2 acc;
	float mass;
	int genSize;
	float genRad;
	int rotationSide;
};

float cTime;
typedef std::chrono::high_resolution_clock Clock;
std::vector <ball> objects;
std::vector <galaxyCenter> galaxies;
ASCIIDrawer Drawer(width, height, xMod, treshold, fisheye);
std::chrono::nanoseconds dt(0);
std::chrono::nanoseconds* dtRef = &dt;
std::chrono::time_point<std::chrono::nanoseconds> tp;
DWORD prev_mode;
std::mutex lockObjects;

void genGalaxy(galaxyCenter& g) {
	float gRad = g.genRad;
	int gStars = g.genSize;
	for (int i = 0; i < gStars; ++i) {
		float p = (1 - pow(sqrt((float)getRandNum(0, 100000) / 25000.0f), 0.05)) * gRad + 0.05f;
		//float p = (rand() % 1000 / 250.0) / 4 * gRad;
		float angle = (rand() % 6284) / 1000.0;
		vec2 pos = {sin(angle) * p, cos(angle) * p};
		pos += g.pos;
		float vmag = pow(G * g.mass / pow(p, Gpow - 1), 0.5);
		//float vmag = pow(G * g.mass / p, 1.0 / 2);
		vec2 vel = { sin(angle + (float)M_PI * g.rotationSide / 2.0f) * vmag, cos(angle + (float)M_PI * g.rotationSide / 2.0f) * vmag };
		//vel *= (1.0f + (rand() % 1000 / 10000 - 0.05));
		vel = g.vel + vel + getRandNum(-velAnom * 5000, velAnom * 5000) / 10000.0;
		ball b = { 1, 0.1, pos, vel, 0, {0,0} };
		objects.push_back(b);
	}
}


void threadPUpdateVels(float& time, const int& start, const int& count, const int& startg, const int& countg) {
	vec2 dir;
	float dirMag;
	for (int i = start; i < start + count; ++i) {
		objects[i].acc.x = 0;
		objects[i].acc.y = 0;
		if (objects[i].fixed) continue;
		for (auto& g : galaxies) {
			dir = (g.pos - objects[i].pos);
			dirMag = dir.magnitude();
			//if (dirMag < 0.05f) continue;
			dir.normalize();
			objects[i].acc += dir * G * g.mass / pow(dirMag, Gpow);
		}
	}
	/*for (int i = start; i < start + count; ++i) {
		objects[i].vel += 0.001 * time * objects[i].acc;
		objects[i].pos += 0.001 * time * objects[i].vel;
	}

	if (keyK.keyDown) 
		for (int i = start; i < start + count; ++i) {
			objects[i].vel += 0.001 * time * objects[i].acc;
			objects[i].pos += 0.001 * time * objects[i].vel;
		}*/
	for (int i = startg; i < startg + countg; ++i) {
		galaxies[i].acc.x = 0;
		galaxies[i].acc.y = 0;
		for (int j = 0; j < galaxies.size(); ++j) {
			if (i == j) continue;
			dir = (galaxies[j].pos - galaxies[i].pos);
			dirMag = dir.magnitude();
			dir.normalize();
			galaxies[i].acc += dir * G * galaxies[j].mass / pow(dirMag, Gpow);
		}
	}

	/*for (int i = startg; i < startg + countg; ++i) {
		galaxies[i].vel += 0.001 * cTime * galaxies[i].acc;
		galaxies[i].pos += 0.001 * cTime * galaxies[i].vel;
	}*/
}

void treadPUpdatePos(float& time) {
	for (int i = 0; i < objects.size(); ++i) {
		objects[i].vel += 0.001 * time * objects[i].acc;
		objects[i].pos += 0.001 * time * objects[i].vel;
	}
	for (int i = 0; i < galaxies.size(); ++i) {
		galaxies[i].vel += 0.001 * cTime * galaxies[i].acc;
		galaxies[i].pos += 0.001 * cTime * galaxies[i].vel;
	}
}

void threadPoolUpdateStatic(bool& trig, const int& oBegin, const int& oEnd, const int& gBegin, const int& gEnd) {//, std::chrono::nanoseconds& dt) {
	while (1) {
		if (trig) {
			auto t = Clock::now();
			threadPUpdateVels(cTime, oBegin, oEnd - oBegin, gBegin, gEnd - gBegin);
			trig = false;
			auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(t - Clock::now());
			//if (!(minRecFrames)) std::this_thread::sleep_until(tp);
			if (!(minRecFrames)) std::this_thread::sleep_for(std::chrono::nanoseconds(diff - *dtRef));
		}
	}
}

void windowCloseHandler(int) {
	BlockInput(false);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), prev_mode);
}

int main()
{
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &prev_mode);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
	signal(SIGINT, windowCloseHandler);
	signal(SIGBREAK, windowCloseHandler);
	auto pt = Clock::now();
	srand(SEED); 
	
	for (int i = 0; i < gCount; ++i) {                                                                              //Generating galaxies
		int k = 0;
		while (!(k == 1 || k == -1))
			k = rand() % 4 - 2;
		int m = (float)(getRandNum(gMassMin, gMassMax));
		galaxyCenter g = { {(float)getRandNum(gPosMin.x, gPosMax.x), (float)getRandNum(gPosMin.y, gPosMax.y)},      //position
			               {(float)getRandNum(gVelMin * 100, gVelMax * 100)/100.0f, (float)getRandNum(gVelMin * 100, gVelMax * 100) / 100.0f},  //velocity
						   {0, 0},                                                                                  //acceleration
						   m,                                                                                       //center mass
						   getRandNum(gSizeMin, gSizeMax),                                                          //stars count in one galaxy
			               m / 10,                                                                                  //radius
		                   k };                                                                                     //rotation side
		genGalaxy(g);
		galaxies.push_back(g);
	}

	/*std::vector<MenuPage> pages;
	std::vector<int> ref1 = { 1, 100, 101 };
	MenuPage1 p1(3, ref1);
	pages.push_back(p1);
	Menu MenuObject(pages, width, height);
	MenuObject.currPage = 0;*/

	float frD = 0;
	int currMenuPos = 0;
	int menuLen = 3;
	int frC = 0;
	bool trig1 = false;
	bool trig2 = false;
	bool trig3 = false;
	bool trig4 = false;
	std::thread thr1(threadPoolUpdateStatic, std::ref(trig1), 0, objects.size() / 3, 0, galaxies.size());
	std::thread thr2(threadPoolUpdateStatic, std::ref(trig2), objects.size() / 3, 2 * objects.size() / 3, 0, 0);
	std::thread thr3(threadPoolUpdateStatic, std::ref(trig3), 2 * objects.size() / 3, 3 * objects.size() / 3, 0, 0);
	bool menu = true;
	bool H = true;
	Drawer.setConsoleSize(width, height);
	Drawer.setConsoleFont(1, 1);
	while (1) {
		if (!menu)
			if (!trig1 and !trig2 and !trig3 and !trig4) {
				//otherWork();
				treadPUpdatePos(cTime);
				trig1 = true;
				trig2 = true;
				trig3 = true;
				//trig4 = true;
			}
			else { 
				//std::this_thread::sleep_for(std::chrono::milliseconds(15));
				continue;
			}
		else std::this_thread::sleep_for(std::chrono::milliseconds(10));
		auto t = Clock::now();
		double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(t - pt).count()/1000000.0;
		pt = t;
		frD += diff;
		cTime = diff*speed;
		frC += 1;
		if (frD > 1000.0 / (float)fps && frC >= minRecFrames) {
			processAll(); // processing all keyboard inputs
			dt = std::chrono::nanoseconds(int(diff * 1000000));
			Drawer.setConsoleSize(width, height);
			Drawer.showNum();
			if (keyP.keyTrigUp || keyEscape.keyTrigUp) menu = !menu;
			if (!menu) {
				if (keyD.keyDown) px += 5.0 / zoom * frD / 50.0;
				if (keyA.keyDown) px -= 5.0 / zoom * frD / 50.0;
				if (keyS.keyDown) py += 5.0 / zoom * frD / 50.0;
				if (keyW.keyDown) py -= 5.0 / zoom * frD / 50.0;
				if (keyQ.keyDown) zoom *= 1.02;
				if (keyE.keyDown) zoom /= 1.02;
				if (keyZ.keyDown) speed += speed * 0.05 * frD / 50.0 + 0.001;
				if (keyX.keyDown) speed -= speed * 0.05 * frD / 50.0 + 0.001;
				if (keyR.keyDown) brightness = clip(brightness + brightness * 0.05 * frD / 50.0, 0.0, 10.0);
				if (keyF.keyDown) brightness = clip(brightness - brightness * 0.05 * frD / 50.0, 0.0, 10.0);
				if (keyT.keyDown) clPower = clip(clPower - 0.005 * frD / 50.0, 0.0, 1.0);
				if (keyG.keyDown) clPower = clip(clPower + 0.005 * frD / 50.0, 0.0, 1.0);
				if (key1.keyDown) brPower -= 0.005 * frD / 50.0;
				if (key2.keyDown) brPower += 0.005 * frD / 50.0;
				if (key3.keyDown) Drawer.fisheye -= 0.005 * Drawer.fisheye;
				if (key4.keyDown) Drawer.fisheye += 0.005 * Drawer.fisheye;
				if (keyH.keyTrigUp) H = !H;
			}
			else {
				if (keyW.keyTrigUp) --currMenuPos;
				if (keyS.keyTrigUp) ++currMenuPos;
				currMenuPos = clip(currMenuPos, 0, menuLen - 1);
			}
			//Drawer.clear();
			clip(clPower, 0.0001f, 1.0f);
			clip(brPower, 0.0001f, 1.0f);
			Drawer.customClear(clPower);
			vec2 coords;
			if (!menu) {
				for (ball* it = objects.data(); it < objects.data() + objects.size(); ++it) {
					float fancyBrightness = brightness * pow(zoom, 0.75); // now all smoothening magic is happening in Drawer.getColor(float col)
					coords = projToScreen(it->pos.x, it->pos.y);
					Drawer.drPointOpaqueNum(coords.x, coords.y, fancyBrightness);
				}
				Drawer.rebright(brPower);
				if (H) {
					for (galaxyCenter *g = galaxies.data(); g < galaxies.data() + galaxies.size(); ++g) {
						Drawer.drCross(((g->pos.x) - px) * Drawer.xMod * zoom,
							((g->pos.y / 2) - py / 2) * zoom,
							8, 0.7, 1);
						Drawer.drArrow(((g->pos.x) - px)* Drawer.xMod* zoom,
							           ((g->pos.y / 2) - py / 2) * zoom, g->vel * zoom * 0.5, 1, 1);
						Drawer.drArrow(((g->pos.x) - px)* Drawer.xMod* zoom,
							           ((g->pos.y / 2) - py / 2) * zoom, g->acc * zoom * 0.5, 1, 4);
					}
					vec2 y = { -height / 2, height / 2 };
					for (int i = (-700) / zoom + px; i < (700) / zoom + px; ++i) {
						Drawer.drLineOpaqueNum((i - px) * zoom * xMod, y.x, (i - px) * zoom * xMod, y.y, 0.6, 3, 0);
					}
					vec2 x = { -width / 2, width / 2 };
					for (int i = -700 / zoom + py; i < 700 / zoom + py; ++i) {
						Drawer.drLineOpaqueNum(x.x, (i - py) * zoom / 2, x.y, (i - py) * zoom / 2, 0.6, 4, 0);
					}
					Drawer.drString("BLUR", 10, 2, 1, 1);
					Drawer.drNumber(clPower, 10, 16, 1, 1, 4);
					Drawer.drString("ZOOM", 10, 34, 1, 1);
					Drawer.drNumber(zoom, 10, 48, 1, 1, 4);
					Drawer.drString("TIME SPEED", 10, 66, 1, 1);
					Drawer.drNumber(speed * 10, 10, 80, 1, 1, 4);
					Drawer.drString("BRIGHTNESS", 10, 98, 1, 1);
					Drawer.drNumber(brightness, 10, 112, 1, 1, 4);
					Drawer.drNumber(brightness, 10, 112, 1, 1, 4);
					Drawer.drString("TOTAL PARTICLE COUNT", 150, 2, 1, 3);
					Drawer.drNumber(objects.size(), 300, 2, 1, 1, 4);
					Drawer.drString("FPS ", 150, 16, 1, 1);
					Drawer.drNumber((float)(1000.0 / (frD)), 180, 16, 1, 1, 4);
					//Drawer.drString("EVALUATED IN ONE FRAME", 280, 16, 1, 1);
					//Drawer.drNumber(frC, 450, 16, 1, 1, 4);
					Drawer.drString("BRIGHTNESS POWER", 10, 130, 1, 1);
					Drawer.drNumber(brPower, 10, 146, 1, 1, 4);
					Drawer.drString("X", 10, 170, 1, 1);
					Drawer.drNumber(px, 40, 170, 1, 1, 4);
					Drawer.drString("Y", 10, 190, 1, 1);
					Drawer.drNumber(py, 40, 190, 1, 1, 4);
					//ball* o = objects.data() + 1;
				}
			}
			else {
				Drawer.drString("PAUSE", 200, 10, 4, 4);
				Drawer.drString("KEY HINT:", 10, 50, 2, 2);
				Drawer.drString("W A S D - TO MOVE", 30, 70, 1, 1);
				Drawer.drString("X Z - TO CHANGE TIME SPEED", 30, 80, 1, 1);
				Drawer.drString("R F - TO CHANGE BRIGHTNESS", 30, 90, 1, 1);
				Drawer.drString("T G - TO CHANGE CLEAR POWER, PREV. FRAME WILL NOT BE", 30, 100, 1, 1);
				Drawer.drString("      REMOVED FULLY, LIKE PIX.COL = (PIX.COL - STRENGTH)", 30, 110, 1, 1);
				Drawer.drString("1 2 - TO CHANGE CLEAR POWER", 30, 120, 1, 1);
				Drawer.drString(" H  - to hide/show text and grid", 30, 130, 1, 1);
				Drawer.drString(" K  - to recalc positions (vel * dt) twice and increase error", 30, 140, 1, 1);
				Drawer.drString("                              for simulating spirals", 30, 150, 1, 1);
				Drawer.drString(" press p (pause button) to start ", 80, 190, 1.5, 1);
			}

			if (colorTest) {
				Drawer.checkPaletteNum();
				Drawer.rebright(brPower);
			}
			frD = 0;
			frC = 0;
		}
	}
}
