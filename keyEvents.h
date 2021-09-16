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

keyState keyA('A');
keyState keyB('B');
keyState keyC('C');
keyState keyD('D');
keyState keyE('E');
keyState keyF('F');
keyState keyG('G');
keyState keyH('H');
keyState keyI('I');
keyState keyJ('J');
keyState keyK('K');
keyState keyL('L');
keyState keyM('M');
keyState keyN('N');
keyState keyO('O');
keyState keyP('P');
keyState keyQ('Q');
keyState keyR('R');
keyState keyS('S');
keyState keyT('T');
keyState keyU('U');
keyState keyV('V');
keyState keyW('W');
keyState keyX('X');
keyState keyY('Y');
keyState keyZ('Z');
keyState key1('1');
keyState key2('2');
keyState key3('3');
keyState key4('4');
keyState key5('5');
keyState key6('6');
keyState key7('7');
keyState key8('8');
keyState key9('9');
keyState key0('0');
keyState keySpace(' ');

void processAll() {
	keyA.keyProcess();
	keyB.keyProcess();
	keyC.keyProcess();
	keyD.keyProcess();
	keyE.keyProcess();
	keyF.keyProcess();
	keyG.keyProcess();
	keyH.keyProcess();
	keyI.keyProcess();
	keyJ.keyProcess();
	keyK.keyProcess();
	keyL.keyProcess();
	keyM.keyProcess();
	keyN.keyProcess();
	keyO.keyProcess();
	keyP.keyProcess();
	keyQ.keyProcess();
	keyR.keyProcess();
	keyS.keyProcess();
	keyT.keyProcess();
	keyU.keyProcess();
	keyV.keyProcess();
	keyW.keyProcess();
	keyX.keyProcess();
	keyY.keyProcess();
	keyZ.keyProcess();
	key1.keyProcess();
	key2.keyProcess();
	key3.keyProcess();
	key4.keyProcess();
	key5.keyProcess();
	key6.keyProcess();
	key7.keyProcess();
	key8.keyProcess();
	key9.keyProcess();
	key0.keyProcess();
	keySpace.keyProcess();
}
