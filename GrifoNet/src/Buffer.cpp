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
#include "Buffer.h"
using namespace grifo;

Buffer::Buffer(unsigned int _bufferSize)
{
	bufferSize = _bufferSize;
	buffer = new char[bufferSize];
	cursor = 0;
	dataSize = 0;
	packet = SDLNet_AllocPacket(bufferSize);
}


Buffer::~Buffer(void)
{
	delete[] buffer;
}

void Buffer::init()
{
	cursor = 0;
	dataSize = 0;
}

bool Buffer::carregar(TCPsocket socket)
{
	dataSize = SDLNet_TCP_Recv(socket, buffer, bufferSize);

	if(dataSize>0)
	{
		cursor = 0;
		return true;
	}

	return false;
}

bool Buffer::carregar(UDPsocket socket)
{
	//SDLNET_MAX_UDPCHANNELS;	//32
	//SDLNET_MAX_UDPADDRESSES;	//4

	SDLNet_UDP_Recv(socket, packet);

	dataSize = packet->len;

	if(dataSize>0)
	{
		memcpy(buffer,packet->data,packet->len);
		cursor = 0;
		return true;
	}
	
	return false;
}

template<typename T> bool Buffer::escrever(T &data)
{
	if(cursor+sizeof(data)<bufferSize-1)
	{
		memcpy(&buffer[cursor],(void*)&data,sizeof(data));
		cursor+=sizeof(data);
		
		dataSize = cursor;
		return true;
	}

	return false;
}

template bool Buffer::escrever<bool>(bool&);
template bool Buffer::escrever<char>(char&);
template bool Buffer::escrever<short>(short&);
template bool Buffer::escrever<int>(int&);
template bool Buffer::escrever<long>(long&);
template bool Buffer::escrever<float>(float&);
template bool Buffer::escrever<double>(double&);

template bool Buffer::escrever<unsigned char>(unsigned char&);
template bool Buffer::escrever<unsigned short>(unsigned short&);
template bool Buffer::escrever<unsigned short const>(unsigned short const&);
template bool Buffer::escrever<Uint16>(Uint16&);
template bool Buffer::escrever<Uint32>(Uint32&);
template bool Buffer::escrever<unsigned int>(unsigned int&);
template bool Buffer::escrever<unsigned long>(unsigned long&);

bool Buffer::escreverBytes(void* data, unsigned int size)
{	
	if(cursor+size<bufferSize-1)
	{
		memcpy(&buffer[cursor],(void*)data,size);
		cursor+=size;
		dataSize = cursor;
		return true;
	}

	return false;
}

void Buffer::setCursor(unsigned int pos)
{
	cursor = ((pos<bufferSize)?pos:bufferSize);
}

template<typename T> bool Buffer::ler(T &data)
{
	if(cursor+sizeof(data)<dataSize/*bufferSize-1*/)
	{
		memcpy((void*)&data,&buffer[cursor],sizeof(data));
		cursor+=sizeof(data);
		return true;
	}

	return false;
}

template bool Buffer::ler<bool>(bool&);
template bool Buffer::ler<char>(char&);
template bool Buffer::ler<short>(short&);
template bool Buffer::ler<int>(int&);
template bool Buffer::ler<long>(long&);
template bool Buffer::ler<float>(float&);
template bool Buffer::ler<double>(double&);

template bool Buffer::ler<unsigned char>(unsigned char&);
template bool Buffer::ler<unsigned short>(unsigned short&);
template bool Buffer::ler<Uint16>(Uint16&);
template bool Buffer::ler<Uint32>(Uint32&);
template bool Buffer::ler<unsigned int>(unsigned int&);
template bool Buffer::ler<unsigned long>(unsigned long&);

bool Buffer::lerBytes(void* data, unsigned int size)
{	
	if(cursor+size<dataSize/*bufferSize-1*/)
	{
		memcpy((void*)data,&buffer[cursor],size);
		cursor+=size;
		return true;
	}

	return false;
}

unsigned int Buffer::getTamanhoDoBuffer()
{
	return bufferSize;
}

const char* Buffer::getDados()
{
	return (const char*) buffer;
}

unsigned int Buffer::getTamanhoDosDados()
{
	return dataSize;
}