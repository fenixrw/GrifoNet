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