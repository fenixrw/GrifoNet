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
#pragma once

#include "Buffer.h"
#include <chrono>

namespace grifo
{

	class ObjetoDeRede
	{
		friend class ControleDeRede;
	public:
		//Increment Object Id
		ObjetoDeRede(void);
		~ObjetoDeRede(void);
	
		virtual void atualizar() = 0;
		virtual unsigned short getTipo() = 0;
	
		virtual void serializar(Buffer *buffer) = 0;
		virtual void deserializar(Buffer *buffer) = 0;
	
		virtual void serializarNovo(Buffer *buffer) = 0;
		virtual void deserializarNovo(Buffer *buffer) = 0;
		virtual void destruir() = 0;

		unsigned short getID();

		bool souDono();
	
		bool usarUDP;
		bool enviar;

		void setFPS(unsigned int fps);

	private:

		void setId(unsigned short _id);
	
		std::chrono::time_point<std::chrono::system_clock> lastSendTime;

		//System variable DON'T CHANGE
		static bool NetworkInstantiate;
		static unsigned short NetworkInstantiateId;
		static unsigned short getNextId();

		bool checkSendTimer();

		bool dono;
		static unsigned short NextId;
		unsigned short id;

		double updateRate;
	};

}