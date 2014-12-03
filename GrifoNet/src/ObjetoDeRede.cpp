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