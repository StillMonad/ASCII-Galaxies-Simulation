#pragma once
#include <WinUser.h>
class keyState
{
public:
	bool keyTrigDown = false;
	bool keyTrigUp = false;
	bool keyDown = false;

	keyState(unsigned short key) {
		this->key = key;
	} 
	void keyProcess() {
		curr = (GetAsyncKeyState(key) & 0x8000);
		trig = curr != prev;
		prev = curr;

		keyTrigUp   = curr == true  && trig;
		keyTrigDown = curr == false && trig;
		keyDown = curr;
	}
private:
	unsigned short key;
	bool curr = false;
	bool trig = false;
	bool prev = false;
};