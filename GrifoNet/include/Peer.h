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