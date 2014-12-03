#include "Peer.h"
using namespace grifo;

Peer::Peer(void)
{
	isSuperPeer=false;
	isRoot=false;
	peerID=0;
	udpChannel=0;

	//Super Peer Only
	numberOfConnections=0;
	acceptingConnections=true;
}

Peer::~Peer(void)
{
}

void Peer::destruirObjetos()
{
	//Destruir Objetos
	for (std::multimap<unsigned short,ObjetoDeRede*>::iterator it=objetos.begin(); it!=objetos.end();)
	{
		(*it).second->destruir();
		ObjetoDeRede *obj = it->second;
		std::multimap<unsigned short,ObjetoDeRede*>::iterator itErase = it;
		it++;
		objetos.erase(itErase);
		delete obj;
	}
}

void Peer::adicionarObjeto(ObjetoDeRede *obj)
{
	objetos.insert(std::pair<unsigned short,ObjetoDeRede*>(obj->getID(),obj));
}

void Peer::atualizarObjetos()
{
	//atualiza objetos de rede
	for (std::map<unsigned short,ObjetoDeRede*>::iterator it=objetos.begin(); it!=objetos.end(); ++it)
	{
		(*it).second->atualizar();
	}
}

////////////////////////////////////////////////////////////////////////////////////////

Grupo::Grupo()
{
	souMembro = true;
	usarSenha = false;
	senha = 0;
	lider = 0;
	id = 0;
}

Grupo::~Grupo()
{
}