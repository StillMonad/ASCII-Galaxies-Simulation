
#define _USE_MATH_DEFINES
#include <iostream>
#include "ASCIIDrawer.h"
#include "vec2.h"
#include "Res.h"
#include "keyEvents.h"
#include <chrono>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <thread>

//==============================================================================================//
//==========================               CONST SECTION               =========================//
//==========================       FEEL FREE TO CHANGE SOMETHING       =========================//
//==============================================================================================//
// max frames per second 
	int   fps    = 25;  
// console width
	int   width  = 600;  
// console height
	int   height = 300;   
// x modifier for making circles circular
	float xMod   = 0.8f;   
// gravity const G
	float G      = 0.5f;    
// initial camera position
	float px = width / 2;
	float py = height / 2 * 2;
// initial zoom
	float zoom = 5.0;
// initial time speed
	float speed = 0.01;
// initial brigtness (brightness is a color intensity multiplier)
	float brightness = 0.02;
// initial clear power (blur) - 1 clears eveverything, 0 clears nothing, something in between create motion blur
	float clPower = 0.8f;
// initial brightness power (pow(brightness, brPower)) can shift colors more to bright or dark
	float brPower = 0.4f;

//===========================    galaxies generation (randomized)     ==========================//
// galaxies count
	int gCount = 2;     
// stars count in one galaxy
	int gSizeMax = 150000;        
	int gSizeMin = 50000; 
// top right bottom left corners of possible positions 
	vec2 gPosMax = {  10,  10 }; 
	vec2 gPosMin = { -10, -10 }; 
// mass of one galaxy center
	int gMassMax = 500;           
	int gMassMin = 100;     
// velocity of galaxies
	int gVelMax  = 3;       
	int gVelMin  = -3;          
// radius of a galaxy
	int gRadMax = 500;
	int gRadMin = 150;

//==============================================================================================//
//===================================   END OF CONST SECTION   =================================//
//==============================================================================================//

int getRandNum(int min, int max) {
	int diff = max - min;
	return (rand() % diff) + min;
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

void genGalaxy(galaxyCenter g) {
	float gRad = g.genRad;
	int gStars = g.genSize;
	for (int i = 0; i < gStars; ++i) {
		float p = (1 - pow(sqrt((float)getRandNum(0, 100000) / 25000.0f), 0.2)) * gRad + 0.05f;
		//float p = (rand() % 1000 / 250.0) / 4 * gRad;
		float angle = (rand() % 6284) / 1000.0;
		vec2 pos = {sin(angle) * p, cos(angle) * p};
		pos += g.pos;
		float vmag = sqrt(G * g.mass / p);
		vec2 vel = { sin(angle + (float)M_PI * g.rotationSide / 2.0f) * vmag, cos(angle + (float)M_PI * g.rotationSide / 2.0f) * vmag };
		//vel *= (1.0f + (rand() % 1000 / 10000 - 0.05));
		vel = g.vel + vel;
		ball b = { 1, 0.1, pos, vel, 0, {0,0} };
		objects.push_back(b);
	}
}


void threadPUpdateFull(float& time, int start, int count, int startg, int countg) {
	for (int i = start; i < start + count; ++i) {
		objects[i].acc = vec2(0, 0);
		if (objects[i].fixed) continue;
		for (int j = 0; j < galaxies.size(); ++j) {
			if (i == j) continue;
			vec2 dir = (galaxies[j].pos - objects[i].pos);
			if (dir.magnitude() < 0.1f) continue;
			dir.normalize();
			objects[i].acc += dir * G * galaxies[j].mass / pow(((objects[i].pos - galaxies[j].pos)).magnitude(), 2);
		}
	}
	for (int i = start; i < start + count; ++i) {
		objects[i].vel += 0.001 * time * objects[i].acc;
		objects[i].pos += 0.001 * time * (objects[i].vel);
	}

	for (int i = startg; i < startg + countg; ++i) {
		galaxies[i].acc = vec2(0, 0);
		for (int j = 0; j < galaxies.size(); ++j) {
			if (i == j) continue;
			vec2 dir = (galaxies[j].pos - galaxies[i].pos);
			dir.normalize();
			galaxies[i].acc += dir * G * galaxies[j].mass / pow(((galaxies[i].pos - galaxies[j].pos)).magnitude(), 2);
		}
	}

	for (int i = startg; i < startg + countg; ++i) {
		galaxies[i].vel += 0.001 * cTime * galaxies[i].acc;
		galaxies[i].pos += 0.001 * cTime * (galaxies[i].vel);
	}
}

void threadPoolUpdate(bool &trig, int threadCount, int threadNum) {
	int s = objects.size() / threadCount;
	int g = galaxies.size() / threadCount;
	while (1) {
		if (trig) {
			threadPUpdateFull(cTime, s * threadNum, s, g * threadNum, g);
			trig = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}

void otherWork() {
	for (int i = 0; i < galaxies.size(); ++i) {
		galaxies[i].acc = vec2(0, 0);
		for (int j = 0; j < galaxies.size(); ++j) {
			if (i == j) continue;
			vec2 dir = (galaxies[j].pos - galaxies[i].pos);
			dir.normalize();
			galaxies[i].acc += dir * G * galaxies[j].mass / pow(((galaxies[i].pos - galaxies[j].pos)).magnitude(), 2);
		}
	}

	for (int i = 0; i < galaxies.size(); ++i) {
		galaxies[i].vel += 0.001 * cTime * galaxies[i].acc;
		galaxies[i].pos += 0.001 * cTime * (galaxies[i].vel);
	}
}

int main()
{
	ASCIIDrawer Drawer(width, height, xMod);
	vec2 a;
	auto pt = Clock::now();
	srand(2); 
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
	
	float frD = 0;
	int currMenuPos = 0;
	int menuLen = 3;
	int frC = 0;
	bool trig1 = false;
	bool trig2 = false;
	bool trig3 = false;
	bool trig4 = false;
	std::thread thr1(threadPoolUpdate, std::ref(trig1), 4, 0);
	std::thread thr2(threadPoolUpdate, std::ref(trig2), 4, 1);
	std::thread thr3(threadPoolUpdate, std::ref(trig3), 4, 2);
	std::thread thr4(threadPoolUpdate, std::ref(trig4), 4, 3);
	bool menu = false;
	bool prMenu = false;
	bool G = true;
	bool prG = false;


	while (1) {
		if (!menu)
			if (!trig1 and !trig2 and !trig3 and !trig4) {
				otherWork();
				trig1 = true;
				trig2 = true;
				trig3 = true;
				trig4 = true;
			}
			else { 
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
		else std::this_thread::sleep_for(std::chrono::milliseconds(30));
		auto t = Clock::now();
		float diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(t - pt).count()/1000000.0;
		pt = t;
		frD += diff;
		cTime = diff*speed;
		frC += 1;
		processAll(); // processing all keyboard inputs
		if (keyC.keyDown) otherWork();
		if (frD > 1000.0 / (float)fps) {
			if (keyP.keyTrigDown) menu = !menu;
			if (!menu) {
				if (keyA.keyDown) px += 5.0 / zoom;
				if (keyD.keyDown) px -= 5.0 / zoom;
				if (keyW.keyDown) py += 5.0 / zoom;
				if (keyS.keyDown) py -= 5.0 / zoom;
				if (keyQ.keyDown) zoom *= 1.01;
				if (keyE.keyDown) zoom /= 1.01;
				if (keyZ.keyDown) speed += speed * 0.05;
				if (keyX.keyDown) speed -= speed * 0.05;
				if (keyR.keyDown) brightness += brightness * 0.05;
				if (keyF.keyDown) brightness -= brightness * 0.05;
				if (keyT.keyDown) clPower -= 0.005;
				if (keyG.keyDown) clPower += 0.005;
				if (key1.keyDown) brPower -= 0.005;
				if (key2.keyDown) brPower += 0.005;
				if (keyH.keyTrigDown) G = !G;
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
			auto it1 = objects.begin();
			auto it2 = galaxies.begin();
			if (!menu) {
				while (it1 != objects.end())
				{
					float fancyBrightness = brightness* pow(zoom, 0.75); // now all smoothening magic is happening in Drawer.getColor(float col)
					Drawer.drPointOpaqueNum(((it1->pos.x * Drawer.xMod - width / 2) + px) * zoom,
						                   ((it1->pos.y / 2 - height / 2) + py / 2) * zoom,    
						                   fancyBrightness);
					++it1;
				}

				Drawer.rebright(brPower);

				if (G) while (it2 != galaxies.end())
				{
					float fancyBrightness = brightness * pow(zoom, 0.75);
					Drawer.drCross(((it2->pos.x * Drawer.xMod - width / 2) + px) * zoom,
						((it2->pos.y / 2 - height / 2) + py / 2) * zoom,
						8, 0.7);
					Drawer.drArrow(((it2->pos.x * Drawer.xMod - width / 2) + px) * zoom,
						((it2->pos.y / 2 - height / 2) + py / 2) * zoom, it2->vel * zoom * 2, 1);
					++it2;
				}
				
				Drawer.drString("BLUR", 10, 2, 1, 1);
				Drawer.drNumber(clPower, 10, 16, 1, 1);
				Drawer.drString("ZOOM", 10, 34, 1, 1);
				Drawer.drNumber(zoom, 10, 48, 1, 1);
				Drawer.drString("TIME SPEED", 10, 66, 1, 1);
				Drawer.drNumber(speed * 10, 10, 80, 1, 1);
				Drawer.drString("BRIGHTNESS", 10, 98, 1, 1);
				Drawer.drNumber(brightness, 10, 112, 1, 1);
				Drawer.drNumber(brightness, 10, 112, 1, 1);
				Drawer.drString("TOTAL PARTICLE COUNT", 150, 2, 1, 1);
				Drawer.drNumber(objects.size(), 300, 2, 1, 1);
				Drawer.drString("FPS ", 150, 16, 1, 1);
				Drawer.drNumber((float)(1000.0/(frD)), 180, 16, 1, 1);
				Drawer.drString("EVALUATED IN ONE FRAME", 280, 16, 1, 1);
				Drawer.drNumber(frC, 450, 16, 1, 1);
				Drawer.drString("BRIGHTNESS POWER", 10, 130, 1, 1);
				Drawer.drNumber(brPower, 10, 146, 1, 1);
				Drawer.drString("X", 10, 170, 1, 1);
				Drawer.drNumber(px, 40, 170, 1, 1);
				Drawer.drString("Y", 10, 190, 1, 1);
				Drawer.drNumber(py, 40, 190, 1, 1);
			}
			else {
				Drawer.clear();
				Drawer.drString("PAUSE", width/2 - 100, 50, 5, 1);
				Drawer.drSprite(menuPos, width / 2 - 100, 100, 20, 5);
				Drawer.drString("SETTINGS", width / 2 - 75, 115, 2, 1);
				Drawer.drSprite(menuPos, width / 2 - 100, 130, 20, 5);
				Drawer.drString("RESUME", width / 2 - 75, 145, 2, 1);
				Drawer.drSprite(menuPos, width / 2 - 100, 160, 20, 5);
				Drawer.drString("EXIT", width / 2 - 75, 175, 2, 1);
				Drawer.drSprite(menuPosSel, width / 2 - 100, 100 + 30 * currMenuPos, 20, 5);
			}
			//Drawer.checkPaletteNum();
			//Drawer.rebright(brPower);
			Drawer.showNum(); 
			frD = 0;
			frC = 0;
		}
	}
}
