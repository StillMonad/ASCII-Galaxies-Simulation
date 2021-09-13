
#define _USE_MATH_DEFINES
#include <iostream>
#include "ASCIIDrawer.h"
#include "vec2.h"
#include "Res.h"
#include <chrono>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <thread>

//==============================================================================================//
//===================================      CONST SECTION       =================================//
//==========================       FEEL FREE TO CHANGE SOMETHING       =========================//
//==============================================================================================//

int fps = 25;
int width = 250;        // console width
int height = 100;       // console height
float xMod = 1.0f;      // x modifier for making circles circular
float G = 0.5f;         // gravity const
int gCount = 2;         // galaxies count

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

std::vector <ball> objects;
std::vector <galaxyCenter> galaxies;

float sigmoid(float x) {
	return 1 / (1 + exp(-x));
}

void genGalaxy(galaxyCenter g) {
	float gRad = g.genRad;
	int gStars = g.genSize;
	for (int i = 0; i < gStars; ++i) {
		float p = (1 - pow(sqrt(rand() % 100000 / 25000.0)/1, 0.2)) * gRad + 0.05f;
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

void genGalaxyTest(galaxyCenter g) {
	float gRad = g.genRad;
	int gStars = g.genSize;
	int count = rand() % 2 + 1;
	for (int i = 0; i < gStars; ++i) {
		float p = (1 - pow(sqrt(rand() % 4000 / 1000.0) / 2, 0.2)) * gRad/2 + 0.2f;
		//float p = (rand() % 1000 / 250.0) / 4 * gRad;
		float angle1 = (rand() % count) * (M_PI*2) / count;
		float angle2 = (rand() % 1000) / 1000.0 * (M_PI * 2 / count) / (p * p);
		vec2 pos = { sin(angle1 + angle2) * p, cos(angle1 + angle2) * p };
		pos += g.pos;
		float vmag = sqrt(G * g.mass / p);
		vec2 vel = { sin(angle2 + angle1 + (float)M_PI * g.rotationSide / 2.0f) * vmag, cos(angle2 + angle1 + (float)M_PI * g.rotationSide / 2.0f) * vmag };
		//vel *= (1.0f + (rand() % 1000 / 10000 - 0.05));
		vel = g.vel + vel;
		ball b = { 1, 0.1, pos, vel, 0, {0,0} };
		objects.push_back(b);
	}
}

float cTime;

void threadPUpdate(float &time, int start, int count) {
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
		//if (cTime == 0.0f) std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (trig) {
			//threadPUpdate(cTime, s * threadNum, s);
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
	typedef std::chrono::high_resolution_clock Clock;
	auto pt = Clock::now();
	srand(2); 
	for (int i = 0; i < gCount; ++i) {                                                         //Generating galaxies
		int k = 0;
		while (!(k == 1 || k == -1))
			k = rand() % 4 - 2;
		int m = (float)(rand() % 200) + 50;
		galaxyCenter g = { {(float)(rand() % 60), (float)(rand() % 60)},                     //position
						   {(float)(rand() % 40 / 5.0), (float)(rand() % 40 / 5.0)},         //velocity
						   {0, 0},                                                             //acceleration
						   m,                                                                  //center mass
						   150000,//rand() % 250000 + 50000,                                            //stars count in one galaxy
			               m/10,                                                               //radius
		                   k };                                                                //rotation side
		//genGalaxyTest(g);
		genGalaxy(g);
		galaxies.push_back(g);
	}
	float px = 276;// 800;
	float py = 180;// 270;
	float zoom = 5.0;
	float speed = 0.01;
	float brightness = 0.02;
	float clPower = 0.8f;
	float brPower = 0.4f;
	float oneSym = 1.0 / Drawer.getColorScheme().size();
	float frD = 0;
	float frD1 = frD;
	float frD2 = frD1;
	int currMenu = 0;
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
	float angTest = 0;
	ShowCursor(FALSE);
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
		float frD1 = frD;
		float frD2 = frD1;
		cTime = diff*speed;
		frC += 1;
		if (GetAsyncKeyState((unsigned short)'C') & 0x8000) otherWork();
		if (frD > 1000.0 / (float)fps) {
			if (!menu) {
				if (GetAsyncKeyState((unsigned short)'A') & 0x8000) px += 5.0 / zoom;
				if (GetAsyncKeyState((unsigned short)'D') & 0x8000) px -= 5.0 / zoom;
				if (GetAsyncKeyState((unsigned short)'W') & 0x8000) py += 5.0 / zoom;
				if (GetAsyncKeyState((unsigned short)'S') & 0x8000) py -= 5.0 / zoom;
				if (GetAsyncKeyState((unsigned short)'Q') & 0x8000) zoom *= 1.01;
				if (GetAsyncKeyState((unsigned short)'E') & 0x8000) zoom /= 1.01;
				if (GetAsyncKeyState((unsigned short)'Z') & 0x8000) speed += speed * 0.05;
				if (GetAsyncKeyState((unsigned short)'X') & 0x8000) speed -= speed * 0.05;
				if (GetAsyncKeyState((unsigned short)'R') & 0x8000) brightness += brightness * 0.05;
				if (GetAsyncKeyState((unsigned short)'F') & 0x8000) brightness -= brightness * 0.05;
				if (GetAsyncKeyState((unsigned short)'T') & 0x8000) clPower -= 0.005;
				if (GetAsyncKeyState((unsigned short)'G') & 0x8000) clPower += 0.005;
				if (GetAsyncKeyState((unsigned short)'1') & 0x8000) brPower -= 0.005;
				if (GetAsyncKeyState((unsigned short)'2') & 0x8000) brPower += 0.005;
				if (GetAsyncKeyState((unsigned short)'P') & 0x8000) {
					prMenu = true;
				}
				else if (prMenu) {
					menu = !menu;
					prMenu = false;
				}
				if (GetAsyncKeyState((unsigned short)'H') & 0x8000) {
					prG = true;
				}
				else if (prG) {
					G = !G;
					prG = false;
				}
			}
			else {
				if (GetAsyncKeyState((unsigned short)'W') & 0x8000) py += 5.0 / zoom;
				if (GetAsyncKeyState((unsigned short)'S') & 0x8000) py -= 5.0 / zoom;
			}
			//Drawer.clear();
			clip(clPower, 0.0001f, 1.0f);
			clip(brPower, 0.0001f, 1.0f);
			Drawer.customClear(clPower);
			//vec2 curs = Drawer.getCursorPos();
			//Drawer.drCircle(curs.x, curs.y, 3, 1, 0, 0.3);
			//float xc = curs.x;
			//float yc = curs.y;
			auto it1 = objects.begin();
			auto it2 = galaxies.begin();
			if (!menu) {
				while (it1 != objects.end())
				{
					//float fancyBrightness = (rand() % 100 / 100.0 * oneSym - oneSym / 2.0) + brightness * pow(zoom, 0.75);
					float fancyBrightness = brightness* pow(zoom, 0.75); // now all smoothening magic is happening in Drawer.getColor(float col)
					Drawer.drPointOpaqueNum(((it1->pos.x * Drawer.xMod - width / 2) + px) * zoom,        // x (on some resolutions required multiply x by some coeff to make circles circle)
						                   ((it1->pos.y / 2 - height / 2) + py / 2) * zoom,    // y
						fancyBrightness);// clip(fancyBrightness * pow(clPower, 0.6), 0.0, 1.0));  // brightness (some magic to make it smooth and zoom-dependent)
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
				
				Drawer.drString("BLURRINESS", 10, 2, 1, 1);
				Drawer.drNumber(1 - clPower, 10, 16, 1, 1);
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
				Drawer.drNumber((float)(1000.0/((frD + frD1 + frD2)/3.0)), 180, 16, 1, 1);
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
				Drawer.drSprite(menuPosSel, width / 2 - 100, 100 + 30 * currMenu, 20, 5);
			}
			//Drawer.checkPaletteNum();
			//Drawer.rebright(brPower);
			Drawer.showNum(); 
			//std::this_thread::sleep_for(std::chrono::milliseconds(3000000));
			frD = 0;
			frC = 0;
		}
	}
}
