/*
========================================================
GrifoNet: P2P Networking library based on SDL_net 2.0
by Felipe Rocha Wagner (2014) 
https://github.com/fenixrw
========================================================

The MIT License (MIT)

Copyright (c) 2014 Felipe Rocha Wagner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "ObjetoDeRede.h"
using namespace grifo;

bool ObjetoDeRede::NetworkInstantiate=false;
unsigned short ObjetoDeRede::NetworkInstantiateId=0;
unsigned short ObjetoDeRede::NextId=0;

unsigned short ObjetoDeRede::getNextId()
{
	NextId++;
	return NextId;
}

bool ObjetoDeRede::souDono()
{
	return dono;
}

ObjetoDeRede::ObjetoDeRede(void)
{
	if(!ObjetoDeRede::NetworkInstantiate)
	{
		dono=true;
		id = ObjetoDeRede::getNextId();
	}
	else
	{
		dono=false;
		id = ObjetoDeRede::NetworkInstantiateId;
	}
	updateRate = 1000/30;
	lastSendTime = std::chrono::system_clock::now();
	usarUDP = false;
	enviar = true;
}

void ObjetoDeRede::setFPS(unsigned int fps)
{
	if(fps>0)
	{
		updateRate = 1000/fps;
	}
	else
	{
		updateRate = 1000/30;
	}
}

bool ObjetoDeRede::checkSendTimer()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSendTime).count();
	
	if(elapsed_ms >= updateRate)
	{
		lastSendTime = now;
		return true;
	}

	return false;
}

void ObjetoDeRede::setId(unsigned short _id)
{
	id=_id;
}


ObjetoDeRede::~ObjetoDeRede(void)
{
}

unsigned short ObjetoDeRede::getID()
{
	return id;
}