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
#include <SDL_net.h>
#include <iostream>
using namespace std;

#include "NetworkMessages.h"
namespace grifo
{

	class Buffer
	{
	public:
		Buffer(unsigned int _bufferSize = 1024);
		~Buffer(void);
		void init();
		bool carregar(TCPsocket socket);
		bool carregar(UDPsocket socket);
		template<typename T> bool escrever(T &data);
		bool escreverBytes(void* data, unsigned int size);
		template<typename T> bool ler(T &data);
		bool lerBytes(void* data, unsigned int size);
		unsigned int getTamanhoDoBuffer();
		const char* getDados();
		unsigned int getTamanhoDosDados();
		void setCursor(unsigned int pos);
	protected:
		UDPpacket *packet;
		char *buffer;
		unsigned int bufferSize;
		int dataSize;
		unsigned int cursor; //write cursor (position to write in the buffer)
	};
}

