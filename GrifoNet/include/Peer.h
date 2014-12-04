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

#include "ObjetoDeRede.h"
#include <map>
#include <set>
#include <list>

namespace grifo
{
	class Peer
	{
		friend class ControleDeRede;
	public:
	
		Peer(void);
		~Peer(void);

	private:
		TCPsocket tcpSock;
		IPaddress tcpAddress;
		IPaddress udpAddressIn;
		IPaddress udpAddressOut;
		unsigned short peerID;
		unsigned int udpChannel;
		bool isSuperPeer;
		bool isRoot;
	
		//Super Peer Only
		unsigned short numberOfConnections;
	
		//Super Peer Only
		bool acceptingConnections;
		
		map<unsigned short,ObjetoDeRede*> objetos;

		void adicionarObjeto(ObjetoDeRede *obj);
		void atualizarObjetos();
		void destruirObjetos();
	

	};

	class Grupo
	{
	public:
		Grupo();
		~Grupo();
		bool souMembro;
		bool usarSenha;
		unsigned short senha;
		unsigned short id;
		unsigned short lider;
		set<unsigned short> membros;
	};
}