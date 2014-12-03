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

