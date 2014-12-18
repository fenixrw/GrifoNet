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
#include "ControleDeRede.h"
using namespace grifo;

#include <tclap\CmdLine.h>
#include <fstream>
#define MAXCONECTIONS 32
#define CONNECTION_SLOTS 20

string DEBUGLOGFILENAME = "debug.log";

#ifdef _DEBUG
#define DEBUGFILE
#endif

string printIp(IPaddress ip)
{
	stringstream sIp;
	Uint32 ipaddr;
	Uint16 port=0;
	port = (Uint16)SDLNet_Read16(&(ip.port));
	ipaddr=SDL_SwapBE32(ip.host);
	/* print out the clients IP and port number */
	sIp <<	(ipaddr>>24) << "." << ((ipaddr>>16)&0xff) << "." << ((ipaddr>>8)&0xff) << "." << (ipaddr&0xff) << ":" << port;
	return sIp.str();
}

PortInfo ControleDeRede::getPortInfo(){

	PortInfo out;

	out.udpIn = _udpInPorta;
	out.udpOut = _udpOutPorta;
	out.tcp = _tcpPorta;

	return out;
}

ControleDeRede::ControleDeRede(void)
{
	_updateSuperPeerInfo = false;
	_primeiraConexao = false;

	_rootPeer = NULL;
	_adminPeer = NULL;

	_souRoot = false;
	_listener = NULL;
	_udpOut = NULL;
	_udpIn = NULL;

	_tcpPorta = 0;
	_tcpPortaN = 0;
	_udpInPorta = 0;
	_udpInPortaN = 0;
	_udpOutPorta = 0;
	_udpOutPortaN = 0;

	_ultimoPeerID = 0;
	_ultimoSuperPeerID = 0;

	_maxPeer = 4;
	_maxSuperPeer = 10;

	_souSuperPeer = false;
	_idSuperPeer = 0;
	_chaveRedeSuperPeer = 0;

	_idPeer = 0;
	_chaveSalaDeJogo=0;

	_fSignalNovoJogador = NULL;
	_fSignalJogadorDesconectou = NULL;

	_fTratarMensagensRecebidas = NULL;
	
	_fAtualizarGrupo = NULL;

	_fCriarObjeto = NULL;
	_dadosCriarObjeto = NULL;
}

ControleDeRede::~ControleDeRede(void)
{
}

void ControleDeRede::inicializar(int argc, char** argv){
	/*TODO - ajustar nomes e descrições dos parametros*/
#pragma region Parse_Params
	//tclap Parse argc+argv
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems.
	try {  

		// Define the command line object, and insert a message
		// that describes the program. The "Command description message" 
		// is printed last in the help text. The second argument is the 
		// delimiter (usually space) and the last one is the version number. 
		// The CmdLine object parses the argv array based on the Arg objects
		// that it contains. 
		TCLAP::CmdLine cmd("libGrifoNet para libUnicornio", ' ', "1.3");
	
		TCLAP::ValueArg<std::string> debugArg("d","debug","debug file name",false,"debug.log","string");
		//TCLAP::ValueArg<std::string> serverIpArg("s","servidor","IP do Servidor Raiz <Root>",false,"127.0.0.1","string");
		//TCLAP::ValueArg<Uint16> severPortArg("p","porta","Porta do Servidor Raiz <Root>",false,2323,"Uint16");
		TCLAP::ValueArg<Uint16> listenerArg("l","listener","Porta do Listener TCP",true,2323,"Uint16");

		TCLAP::ValueArg<unsigned int> maxPeerArg("p","maxPeer","Maximun number of Peers in a Game Room",false,4,"uint");
		TCLAP::ValueArg<unsigned int> maxSuperPeerArg("g","maxSuperPeer","Maximun number of Super Peers (Game Rooms) in the system",false,10,"uint");

		// Add the argument nameArg to the CmdLine object. The CmdLine object
		// uses this Arg to parse the command line.
		cmd.add( debugArg );
		//cmd.add( serverIpArg );
		//cmd.add( severPortArg );
		cmd.add( listenerArg );

		// Define a switch and add it to the command line.
		// A switch arg is a boolean argument and only defines a flag that
		// indicates true or false.  In this example the SwitchArg adds itself
		// to the CmdLine object as part of the constructor.  This eliminates
		// the need to call the cmd.add() method.  All args have support in
		// their constructors to add themselves directly to the CmdLine object.
		// It doesn't matter which idiom you choose, they accomplish the same thing.
		TCLAP::SwitchArg rootSwitch("r","root","Iniciar como Servidor Raiz <Root>", cmd, false);
		//TCLAP::SwitchArg serverSwitch("s","startServer","Iniciar como Servidor (Game Room)", cmd, false);

		// Parse the argv array.
		cmd.parse( argc, argv );

		// Get the value parsed by each arg. 
		_tcpPorta = listenerArg.getValue();
		SDLNet_Write16(_tcpPorta,&_tcpPortaN);

		_souRoot = rootSwitch.getValue();

		if(debugArg.isSet())
		{
			DEBUGLOGFILENAME = debugArg.getValue();
		}

		if(maxPeerArg.isSet())
		{
			_maxPeer = maxPeerArg.getValue();
		}

		if(maxSuperPeerArg.isSet())
		{
			_maxSuperPeer = maxSuperPeerArg.getValue();
		}

	} 
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
	}

#pragma endregion

#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::trunc);
#endif

	_primeiraConexao = _souRoot;
	if(_souRoot)
	{
		srand((unsigned)time(NULL));
		_chaveRedeSuperPeer = rand();
		_chaveSalaDeJogo = rand();
		_souSuperPeer = true;
	}

#pragma region SDLNet_Init
	
	if (SDLNet_Init() == -1)
    {
#ifdef DEBUGFILE
        if(debugFile.is_open())
			debugFile << "Failed to intialise SDL_net: " << SDLNet_GetError() << endl;
#endif
        exit(-1); // Quit!
    }
	
#pragma endregion

#pragma region TCP_Open
	/* Resolve the argument into an IPaddress type */
	if(SDLNet_ResolveHost(&_listenerIp,NULL,_tcpPorta)==-1)//"127.0.0.1"
	{
#ifdef DEBUGFILE
		if(debugFile.is_open())
			debugFile << "SDLNet_ResolveHost:" << SDLNet_GetError() << "\n";
#endif
		SDLNet_Quit();
		SDL_Quit();
		exit(-3);
	}
	
	 // Try to open the server socket
	_listener = SDLNet_TCP_Open(&_listenerIp);
 
    if (!_listener)
    {
#ifdef DEBUGFILE
        if(debugFile.is_open())
			debugFile << "Failed to open the server socket: " << SDLNet_GetError() << "\n";
#endif
        exit(-4);
    }

#pragma endregion

#pragma region UDP_Open
	_udpIn = SDLNet_UDP_Open(0);
	if(!_udpIn)
	{
		exit(-33);
	}
	_udpInPortaN = SDLNet_UDP_GetPeerAddress(_udpIn,-1)->port;
	_udpInPorta = (Uint16) SDLNet_Read16(&_udpInPortaN);

	_udpOut = SDLNet_UDP_Open(0);
	if(!_udpOut)
	{
		exit(-33);
	}
	_udpOutPortaN = SDLNet_UDP_GetPeerAddress(_udpOut,-1)->port;
	_udpOutPorta = (Uint16) SDLNet_Read16(&_udpOutPortaN);
#pragma endregion
		
#pragma region SocketSet
	_socketSet = SDLNet_AllocSocketSet(CONNECTION_SLOTS+_maxSuperPeer+_maxPeer+2);//in & out connections + superPeers + Peers + listener + udpIn
	SDLNet_TCP_AddSocket(_socketSet,_listener);
	SDLNet_UDP_AddSocket(_socketSet,_udpIn);
#pragma endregion

#ifdef DEBUGFILE
    if(debugFile.is_open())
	{
		debugFile << "##################" << (_souRoot?" ROOT <tcp=":" USER <tcp=") << _tcpPorta << "> <udpIn=" << _udpInPorta << "> <udpOut=" << _udpOutPorta << ">###########################" << endl;
		debugFile.close();
	}
#endif

}

void ControleDeRede::finalizar(){
	// Free our socket set (i.e. all the clients in our socket set)
	SDLNet_FreeSocketSet(_socketSet);
 
	//Close all other open sockets
	desconectar();
    
	// Close our server socket, cleanup SDL_net and finish!
	SDLNet_TCP_Close(_listener);
	SDLNet_UDP_Close(_udpIn);
	SDLNet_UDP_Close(_udpOut);

	

	SDLNet_Quit();
}

void ControleDeRede::adicionarObjeto(ObjetoDeRede* obj){
	obj->dono = true;
	_objetos.insert(std::pair<unsigned short,ObjetoDeRede*>(obj->getID(),obj));
	enviarMensagemInstanciarObjeto(obj);
}

bool ControleDeRede::souRoot()
{
	return _souRoot;
}

bool ControleDeRede::souSuperPeer()
{
	return _souSuperPeer;
}

unsigned short ControleDeRede::getIdPeer()
{
	return _idPeer;
}

void ControleDeRede::conectar(string host, Uint16 port, bool comoSuperPeer){
	IPaddress tempIP;
	if(SDLNet_ResolveHost(&tempIP, host.c_str(), port)==-1)
	{
		string msg = "Error: Failed to resolve the server hostname to an IP address.";
		//TODO: Set Error
		return;
	}
	else
	{
			conectar(tempIP,comoSuperPeer);
	}
}

void ControleDeRede::conectar(IPaddress ip, bool comoSuperPeer)
{
	if (_conexoesSolicitadas.size() < CONNECTION_SLOTS/2)//(peers.size() < maxPeers)
    { 
		TCPsocket tempSocket = SDLNet_TCP_Open(&ip);

		if(!tempSocket)
		{
			string msg = "Error: Failed to open connection to server.";
			msg += SDLNet_GetError();
			//TODO: Set Error
#ifdef DEBUGFILE
			ofstream debugFile;
			debugFile.open(DEBUGLOGFILENAME,std::ios::trunc);
			if(debugFile.is_open())
			{
				debugFile << msg << " Failed TCP_Open for address [" << printIp(ip) << "]" << endl;
				debugFile.close();
			}
#endif
			return;
		}
		else
		{
			SDLNet_TCP_AddSocket(_socketSet, tempSocket);
			Peer tempPeer;
			tempPeer.tcpSock = tempSocket;
			tempPeer.tcpAddress = ip;
			tempPeer.udpAddressIn.host = SDLNet_TCP_GetPeerAddress(tempPeer.tcpSock)->host;
			tempPeer.udpAddressIn.port = 0;
			tempPeer.udpAddressOut.host = tempPeer.udpAddressIn.host;
			tempPeer.udpAddressOut.port = 0;
			tempPeer.udpChannel = 0;
			tempPeer.peerID = 0;
			tempPeer.isRoot = false;
			tempPeer.isSuperPeer = false;

			if(comoSuperPeer)
			{
				tempPeer.isSuperPeer=true;
			}

			_conexoesSolicitadas.push_back(tempPeer);
			
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Conexão [" << printIp(ip) <<"] adicionada a lista de solicitações" << endl;
		debugFile.close();
	}
#endif		
			// Wait for up to five seconds for a response from the server.
			// Note: If we don't check the socket set and WAIT for the response, we'll be checking before the
			// server can respond, and it'll look as if the server sent us nothing back!
			//int activeSockets = SDLNet_CheckSockets(tcpSockSet, 5000);
			//SDLNet_GetLocalAddresses
			
		}
	}
	else
	{
		string msg = "Error: Connection Socket Set is FULL.";
		//TODO: Set Error
#ifdef DEBUGFILE
			ofstream debugFile;
			debugFile.open(DEBUGLOGFILENAME,std::ios::trunc);
			if(debugFile.is_open())
			{
				debugFile << msg << " Failed TCP_Open for address [" << printIp(ip) << "]" << endl;
				debugFile.close();
			}
#endif
		return;
	}
}

void ControleDeRede::desconectar()
{
	if(_souRoot)
	{
		map<unsigned short, Peer>::iterator it;

		if(_souSuperPeer)
		{
			it= _superPeers.begin();
			while(it !=_superPeers.end())
			{
				SDLNet_TCP_DelSocket(_socketSet,it->second.tcpSock);
				SDLNet_TCP_Close(it->second.tcpSock);
				map<unsigned short, Peer>::iterator itErase = it;
				it++;
				_superPeers.erase(itErase);
			}
		}

	}
	else 
	{
		map<unsigned short, Peer>::iterator it;

		if(_souSuperPeer)
		{
			it= _superPeers.begin();
			while(it !=_superPeers.end())
			{
				SDLNet_TCP_DelSocket(_socketSet,it->second.tcpSock);
				SDLNet_TCP_Close(it->second.tcpSock);
				map<unsigned short, Peer>::iterator itErase = it;
				it++;
				_superPeers.erase(itErase);
			}
		}

		it= _peers.begin();
		while(it !=_peers.end())
		{
			SDLNet_TCP_DelSocket(_socketSet,it->second.tcpSock);
			SDLNet_TCP_Close(it->second.tcpSock);
			SDLNet_UDP_Unbind(_udpIn,it->second.udpChannel);
			SDLNet_UDP_Unbind(_udpOut,it->second.udpChannel);
			map<unsigned short, Peer>::iterator itErase = it;
			it++;
			_peers.erase(itErase);
		}
	}
		
	_souRoot = false;		
	_souSuperPeer = false;
	_updateSuperPeerInfo = false;
	_primeiraConexao = false;
	_idPeer = 0;
	_idSuperPeer = 0;

	_rootPeer = NULL;
	_adminPeer = NULL;

}

void ControleDeRede::atualizar(){

	int numActiveSockets = SDLNet_CheckSockets(_socketSet, 0);
	if(numActiveSockets>0)
	{
		receberConexoes();
		tratarMensagens();
	}

	if(!_souRoot)
	{
		atualizarObjetos();

		std::map<unsigned short,Peer>::iterator it=_peers.begin();
		while ( it != _peers.end())
		{
			it->second.atualizarObjetos();
			it++;
		}
	
		if(_souSuperPeer)
		{
			//atualizar dados do SuperPeer
			if(_rootPeer && _updateSuperPeerInfo)
			{
				buffer.init();
				uMessageHeader header;
				header.header.info.version=1;
				header.header.info.type=MessageType_CONTROL;
				header.header.info.extension=0;
				header.header.info.AddresseeType=AddresseeType_SUPERPEER;
				header.header.info.flags=FLAG_ATUALIZAR;
				header.header.addresseeID=_rootPeer->peerID;
				header.header.senderID=_idSuperPeer;
				buffer.escreverBytes(header.stream,sizeof(header));
				unsigned short numberOfPeers = _peers.size();
				buffer.escrever(numberOfPeers);

				SDLNet_TCP_Send(_rootPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);

#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "[SP] - Update [" << numberOfPeers << "]" << endl;
		debugFile.close();
	}
#endif

				_updateSuperPeerInfo = false;
			}
		}
	}


}

bool ControleDeRede::enviarMensagemPeer(unsigned short peerID, Buffer &message, bool udp){

	std::map<unsigned short, Peer>::iterator it = _peers.find(peerID);

	if(it != _peers.end())
	{
		buffer.init();
		uMessageHeader header;
		header.header.info.version=1;
		header.header.info.type=MessageType_COMMON;
		header.header.info.extension=0;
		header.header.info.AddresseeType=AddresseeType_PEER;
		header.header.info.flags=0;
		header.header.addresseeID=it->second.peerID;
		header.header.senderID=_idPeer;
		buffer.escreverBytes(header.stream,sizeof(header));
		buffer.escreverBytes((void*)message.getDados(),message.getTamanhoDosDados()+1);

		if(udp)
		{
			UDPpacket *packet = SDLNet_AllocPacket(buffer.getTamanhoDoBuffer());
			memcpy(packet->data,buffer.getDados(),buffer.getTamanhoDosDados()+1);
			packet->len = buffer.getTamanhoDosDados()+1;
			SDLNet_UDP_Send(_udpOut,it->second.udpChannel,packet);
			SDLNet_FreePacket(packet);
			return true;
		}
		else
		{
			SDLNet_TCP_Send(it->second.tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
			return true;
		}
	}

	return false;
}

bool ControleDeRede::enviarMensagemGrupo(unsigned short grupoID, Buffer &message, bool udp){
	
	std::map<unsigned short, Grupo>::iterator it = _grupos.find(grupoID);

	if(it != _grupos.end())
	{
		if(it->second.souMembro)
		{
			buffer.init();
			uMessageHeader header;
			header.header.info.version=1;
			header.header.info.type=MessageType_COMMON;
			header.header.info.extension=0;
			header.header.info.AddresseeType=AddresseeType_GROUP;
			header.header.info.flags=0;
			header.header.addresseeID=grupoID;
			header.header.senderID=_idSuperPeer;
			buffer.escreverBytes(header.stream,sizeof(header));
			buffer.escreverBytes((void*)message.getDados(),message.getTamanhoDosDados()+1);
		
			if(udp)
			{
				UDPpacket *packet = SDLNet_AllocPacket(buffer.getTamanhoDoBuffer());
				memcpy(packet->data,buffer.getDados(),buffer.getTamanhoDosDados()+1);
				packet->len = buffer.getTamanhoDosDados()+1;

				//for each peer in the group
				for(std::set<unsigned short>::iterator idList=it->second.membros.begin(); idList!=it->second.membros.end(); idList++)
				{
					//avoid sending to myself
					if((*idList) == _idPeer)
					{
						continue;
					}

					std::map<unsigned short, Peer>::iterator itP = _peers.find((*idList));

					if(itP != _peers.end())
					{		
						SDLNet_UDP_Send(_udpOut,itP->second.udpChannel,packet);
					}
				}
				SDLNet_FreePacket(packet);
				return true;
			}
			else
			{
				//for each peer in the group
				for(std::set<unsigned short>::iterator idList=it->second.membros.begin(); idList!=it->second.membros.end(); idList++)
				{
					//avoid sending to myself
					if((*idList) == _idPeer)
					{
						continue;
					}
				
					std::map<unsigned short, Peer>::iterator itP = _peers.find((*idList));

					if(itP != _peers.end())
					{		
						SDL_Delay(15);
						SDLNet_TCP_Send(itP->second.tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						SDL_Delay(15);
					}
				}
				return true;
			}
		}
	}
	return false;
}

void ControleDeRede::grupoCriar(unsigned short grupoID, vector<unsigned short> membrosPeerID)
{
	if(!_souRoot)
	{
		buffer.init();
		uMessageHeader header;
		header.header.info.version=1;
		header.header.info.type=MessageType_CONTROL;
		header.header.info.extension=0;
		header.header.info.AddresseeType=AddresseeType_SUPERPEER;
		header.header.info.flags=FLAG_GRUPO;
		header.header.addresseeID=_idSuperPeer;
		header.header.senderID=_idPeer;
		if(!_souSuperPeer)
		{
			buffer.escreverBytes(header.stream,sizeof(header));
		}

		uGroupMessageHeader gHeader;
		gHeader.header.groupId = grupoID;
		gHeader.header.type = GroupMessageType_Create;
		unsigned short numberOfMembers = membrosPeerID.size();

		if(membrosPeerID.size() > 0)
		{
			gHeader.header.flags = Flag_Group_PEERS;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
				
			buffer.escrever(numberOfMembers);

			for(int i = 0; i < numberOfMembers; i++)
			{
				buffer.escrever(membrosPeerID[i]);
			}
		}
		else
		{
			gHeader.header.flags = 0;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
		}

		if(_souSuperPeer)
		{
			char endByte = '\n';
			buffer.escrever(endByte);
			buffer.setCursor(0);
			tratarMensagemControleGrupo(header.header,buffer);
		}
		else
		{
			if(_adminPeer)
			{
				SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
			}
			else
			{
				//search admin
				for(map<unsigned short, Peer>::iterator it = _peers.begin(); it != _peers.end(); it++)
				{
					if(it->second.isSuperPeer)//TODO: check if is acceptingConnections
					{
						_adminPeer = &(it->second);
						SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						break;
					}
				}
			}
		}
	}
}

void ControleDeRede::grupoEntrar(unsigned short grupoID)
{
	vector<unsigned short> emptyMembers;
	grupoAdicionarMembros(grupoID,emptyMembers);
}

void ControleDeRede::grupoAdicionarMembros(unsigned short grupoID, vector<unsigned short> membrosPeerID){

	if(!_souRoot)
	{
		buffer.init();
		uMessageHeader header;
		header.header.info.version=1;
		header.header.info.type=MessageType_CONTROL;
		header.header.info.extension=0;
		header.header.info.AddresseeType=AddresseeType_SUPERPEER;
		header.header.info.flags=FLAG_GRUPO;
		header.header.addresseeID=_idSuperPeer;
		header.header.senderID=_idPeer;
		if(!_souSuperPeer)
		{
			buffer.escreverBytes(header.stream,sizeof(header));
		}
		uGroupMessageHeader gHeader;
		gHeader.header.groupId = grupoID;
		gHeader.header.type = GroupMessageType_Add;
		unsigned short numberOfMembers = membrosPeerID.size();
		if(membrosPeerID.size() > 0)
		{
			gHeader.header.flags = Flag_Group_PEERS;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
				
			buffer.escrever(numberOfMembers);
			for(int i = 0; i < numberOfMembers; i++)
			{
				buffer.escrever(membrosPeerID[i]);
			}
		}
		else
		{
			gHeader.header.flags = 0;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
		}

		if(_souSuperPeer)
		{
			char endByte = '\n';
			buffer.escrever(endByte);
			buffer.setCursor(0);
			tratarMensagemControleGrupo(header.header,buffer);
		}
		else
		{			
			if(_adminPeer)
			{
				SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
			}
			else
			{
				//search admin
				for(map<unsigned short, Peer>::iterator it = _peers.begin(); it != _peers.end(); it++)
				{
					if(it->second.isSuperPeer)//TODO: check if is acceptingConnections
					{
						_adminPeer = &(it->second);
						SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						break;
					}
				}
			}
		}
	}
}

void ControleDeRede::grupoSair(unsigned short grupoID){
	vector<unsigned short> emptyMembers;
	grupoExcluirMembros(grupoID,emptyMembers);
}

void ControleDeRede::grupoExcluirMembros(unsigned short grupoID, vector<unsigned short> membrosPeerID){
	/*TODO*/
	if(!_souRoot)
	{
		buffer.init();
		uMessageHeader header;
		header.header.info.version=1;
		header.header.info.type=MessageType_CONTROL;
		header.header.info.extension=0;
		header.header.info.AddresseeType=AddresseeType_SUPERPEER;
		header.header.info.flags=FLAG_GRUPO;
		header.header.addresseeID=_idSuperPeer;
		header.header.senderID=_idPeer;
		if(!_souSuperPeer)
		{
			buffer.escreverBytes(header.stream,sizeof(header));
		}
		uGroupMessageHeader gHeader;
		gHeader.header.groupId = grupoID;
		gHeader.header.type = GroupMessageType_Remove;
		unsigned short numberOfMembers = membrosPeerID.size();
		if(membrosPeerID.size() > 0)
		{
			gHeader.header.flags = Flag_Group_PEERS;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
				
			buffer.escrever(numberOfMembers);
			for(int i = 0; i < numberOfMembers; i++)
			{
				buffer.escrever(membrosPeerID[i]);
			}
		}
		else
		{
			gHeader.header.flags = 0;
			buffer.escreverBytes(gHeader.stream,sizeof(gHeader));
		}

		if(_souSuperPeer)
		{
			char endByte = '\n';
			buffer.escrever(endByte);
			buffer.setCursor(0);
			tratarMensagemControleGrupo(header.header,buffer);
		}
		else
		{
			if(_adminPeer)
			{
				SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
			}
			else
			{
				//search admin
				for(map<unsigned short, Peer>::iterator it = _peers.begin(); it != _peers.end(); it++)
				{
					if(it->second.isSuperPeer)//TODO: check if is acceptingConnections
					{
						_adminPeer = &(it->second);
						SDLNet_TCP_Send(_adminPeer->tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						break;
					}
				}
			}
		}
	}
}

//Private

void ControleDeRede::tratarMensagemControleGrupo(MessageHeader header, Buffer &message)
{
	if(!_souRoot)
	{
		/*TODO*/
		uGroupMessageHeader gHeader;
		if(message.lerBytes(gHeader.stream,sizeof(gHeader)))
		{
			//encontrar grupo
			std::map<unsigned short, Grupo>::iterator itG = _grupos.find(gHeader.header.groupId);
			bool atualizarGrupo = false;
#pragma region Peer_TratarResposta
			if(checkFlag(gHeader.header.flags, Flag_Group_RESPONSE))//se é uma resposta
			{
				//setar flag souMembro como false para grupos onde o _superPeer não está incluso
				//g.souMembro = false;
#pragma region Peer_GroupREFUSE
				if(checkFlag(gHeader.header.flags, Flag_Group_REFUSE))
				{
					//send Group Error
					Grupo g;
					g.id = gHeader.header.groupId;
					g.souMembro = false;
					if(_fAtualizarGrupo)
					{
						_fAtualizarGrupo(g);
					}
				}
#pragma endregion
#pragma region Peer_GroupADD
				else if(gHeader.header.type == GroupMessageType_Add)
				{
					if(itG != _grupos.end())
					{
						if(itG->second.souMembro)
						{
							atualizarGrupo = true;
						}
						
						if(checkFlag(gHeader.header.flags, Flag_Group_PEERS))
						{
							unsigned short memberId=0, numberOfMembers=0;
							if(message.ler(numberOfMembers))
							{
								for(int i=0; i< numberOfMembers; i++)
								{
									if(message.ler(memberId))
									{
										if(memberId == _idPeer)
										{
											atualizarGrupo = true;
											itG->second.souMembro = true;
										}

										itG->second.membros.insert(memberId);
									}
								}
							}
						}
					}
				}
#pragma endregion
#pragma region Peer_GroupREMOVE
				else if(gHeader.header.type == GroupMessageType_Remove)
				{
					if(itG != _grupos.end())
					{
						if(itG->second.souMembro)
						{
							atualizarGrupo = true;
						}
						
						if(checkFlag(gHeader.header.flags, Flag_Group_PEERS))
						{
							unsigned short memberId=0, numberOfMembers=0;
							if(message.ler(numberOfMembers))
							{
								for(int i=0; i< numberOfMembers; i++)
								{
									if(message.ler(memberId))
									{
										if(memberId == _idPeer)
										{
											atualizarGrupo = true;
											itG->second.souMembro = false;
										}

										//itG->second.membros.remove_if([memberId](unsigned short n){ return n == memberId; });
										set<unsigned short>::iterator itM = itG->second.membros.find(memberId);
										if(itM != itG->second.membros.end())
										{
											itG->second.membros.erase(itM);
										}
									}
								}
							}
						}
						else //Remover o Grupo
						{
							Grupo g;
							g.id = gHeader.header.groupId;
							g.souMembro = false;
							if(_fAtualizarGrupo)
							{
								_fAtualizarGrupo(g);
							}
							_grupos.erase(itG);
						}
					}
				}
#pragma endregion
#pragma region Peer_GroupCREATE
				else if(gHeader.header.type == GroupMessageType_Create)
				{
					if(itG == _grupos.end())
					{
						//Criar Grupo
						Grupo g;
						g.souMembro = false;
						g.id = gHeader.header.groupId;

						if(checkFlag(gHeader.header.flags, Flag_Group_PASSWORD))
						{
							unsigned short password = 0;
							g.usarSenha = true;
							message.ler(g.senha);
						}

						if(checkFlag(gHeader.header.flags, Flag_Group_PEERS))
						{
							unsigned short memberId=0, numberOfMembers=0;
							if(message.ler(numberOfMembers))
							{
								for(int i=0; i< numberOfMembers; i++)
								{
									if(message.ler(memberId))
									{
										if(i==0)
										{
											g.lider = memberId;
										}
										if(memberId == _idPeer)
										{
											atualizarGrupo = true;
											g.souMembro = true;
										}

										g.membros.insert(memberId);
									}
								}
							}
							_grupos.insert(std::pair<unsigned short, Grupo>(g.id,g));
							itG = _grupos.find(g.id);
						}
					}
				}
#pragma endregion
			}
#pragma endregion							
#pragma region SuperPeer_TratarRequisicao
			else if(_souSuperPeer) //tratar requisições de Grupo (somente SUperPeer)
			{
				//alterar ou adicionar à lista de grupos
				//find(list.begin(),list.end(),_idPeer);
				//enviar para todos os membros (exceto o _superPeer - verificar _idPeer)						
#pragma region SuperPeer_RequestGroupADD
				if(gHeader.header.type == GroupMessageType_Add)
				{
					if(itG != _grupos.end())
					{
						list<unsigned short> newMembers;
						bool error = false;

						if(checkFlag(gHeader.header.flags, Flag_Group_PEERS))
						{
							/*TODO*/
							if(header.senderID == itG->second.lider)//only lider can add others besides self
							{
								unsigned short numberOfMembers = 0;
								message.ler(numberOfMembers);

								for(int i=0; i<numberOfMembers;i++)
								{
									unsigned short membroId = 0;
									if(message.ler(membroId))
									{
										if(std::find(itG->second.membros.begin(),itG->second.membros.end(),membroId) == itG->second.membros.end())
										{
											newMembers.push_back(membroId);
										}
									}
								}
							}
							else
							{
								//somente o lider pode adicionar membros além dele mesmo
								error = true;
							}
						}
						else
						{
							if(std::find(itG->second.membros.begin(),itG->second.membros.end(),header.senderID) == itG->second.membros.end())
							{
								newMembers.push_back(header.senderID);
							}
							else
							{
								//player already added
								error = true;
							}
						}

						if(!error && newMembers.size()>0)
						{
							//SEND MESSAGE
							message.init();
							uMessageHeader header2;
							header2.header.info.version=1;
							header2.header.info.type=MessageType_CONTROL;
							header2.header.info.extension=0;
							header2.header.info.AddresseeType=AddresseeType_GROUP;
							header2.header.info.flags=FLAG_GRUPO;
							header2.header.addresseeID=itG->second.id;
							header2.header.senderID=_idPeer;

							message.escreverBytes(header2.stream,sizeof(header2));

							uGroupMessageHeader gHeader2;
							gHeader2.header.flags = Flag_Group_PEERS | Flag_Group_RESPONSE;
							gHeader2.header.type = GroupMessageType_Add;
							gHeader2.header.groupId = itG->second.id;

							message.escreverBytes(gHeader2.stream,sizeof(gHeader2));

							unsigned short numberOfMembers = newMembers.size();

							message.escrever(numberOfMembers);
														
							for(list<unsigned short>::iterator itN = newMembers.begin(); itN != newMembers.end(); itN++)
							{
								message.escrever(*itN);
							}

							for(set<unsigned short>::iterator itM = itG->second.membros.begin(); itM != itG->second.membros.end(); itM++)
							{
								if(*itM==_idPeer)//The super Peer is member, must update
								{
									itG->second.souMembro = true;
									atualizarGrupo = true;
								}
								else
								{
									//send message
									std::map<unsigned short,Peer>::iterator itP = _peers.find(*itM);
									if(itP != _peers.end())
									{
										SDLNet_TCP_Send(itP->second.tcpSock,message.getDados(), message.getTamanhoDosDados()+1);
									}
									else
									{
										//TOTO: <ERROR> Peer not found
									}
								}
							}
							//adicionar novos membros							
							for(list<unsigned short>::iterator itN = newMembers.begin(); itN != newMembers.end(); itN++)
							{
								itG->second.membros.insert(*itN);
							}

							//Send New Members
							
							message.init();
							message.escreverBytes(header2.stream,sizeof(header2));
							gHeader2.header.type = GroupMessageType_Create;
							message.escreverBytes(gHeader2.stream,sizeof(gHeader2));
							numberOfMembers = itG->second.membros.size();

							message.escrever(numberOfMembers);
														
							for(set<unsigned short>::iterator itM = itG->second.membros.begin(); itM != itG->second.membros.end(); itM++)
							{
								message.escrever(*itM);
							}
																				
							for(list<unsigned short>::iterator itN = newMembers.begin(); itN != newMembers.end(); itN++)
							{
								if(*itN==_idPeer)//The super Peer is member, must update
								{
									itG->second.souMembro = true;
									atualizarGrupo = true;
								}
								else
								{
									//send message
									std::map<unsigned short,Peer>::iterator itP = _peers.find(*itN);
									if(itP != _peers.end())
									{
										SDLNet_TCP_Send(itP->second.tcpSock,message.getDados(), message.getTamanhoDosDados()+1);
									}
									else
									{
										//TOTO: <ERROR> Peer not found
									}
								}

							}
						}


					}
				}
#pragma endregion							
#pragma region SuperPeer_RequestGroupRemove
				else if(gHeader.header.type == GroupMessageType_Remove)
				{
					if(itG != _grupos.end())
					{
						/*TODO*/
						set<unsigned short> excludedMembers;
						bool error = false;
						if(checkFlag(gHeader.header.flags,Flag_Group_PEERS))
						{
							if(header.senderID == itG->second.lider)
							{
								unsigned short numberOfMembers = 0;
								message.ler(numberOfMembers);

								for(int i=0; i<numberOfMembers;i++)
								{
									unsigned short membroId = 0;
									if(message.ler(membroId))
									{
										set<unsigned short>::iterator itM = itG->second.membros.find(membroId);

										if(itM != itG->second.membros.end())
										{
											itG->second.membros.erase(itM);
										}
										excludedMembers.insert(membroId);
									}
								}
							}
							else
							{
								//somente o lider pode excluir membros do grupo
								error = true;
							}
						}
						else
						{
							if(header.senderID == itG->second.lider)
							{
								excludedMembers = itG->second.membros;
								itG->second.membros.clear();
							}
							else
							{
								excludedMembers.insert(header.senderID);
							}
						}

						//SEND MESSAGE
						if(!error && excludedMembers.size()>0)
						{
							//Send Message to Current Players (if exist)
							bool groupDeleted = false;
							if(itG->second.membros.size()>0)
							{
								message.init();
								uMessageHeader header2;
								header2.header.info.version=1;
								header2.header.info.type=MessageType_CONTROL;
								header2.header.info.extension=0;
								header2.header.info.AddresseeType=AddresseeType_GROUP;
								header2.header.info.flags=FLAG_GRUPO;
								header2.header.addresseeID=itG->second.id;
								header2.header.senderID=_idPeer;

								message.escreverBytes(header2.stream,sizeof(header2));

								uGroupMessageHeader gHeader2;
								gHeader2.header.flags = Flag_Group_PEERS | Flag_Group_RESPONSE;
								gHeader2.header.type = GroupMessageType_Remove;
								gHeader2.header.groupId = itG->second.id;

								message.escreverBytes(gHeader2.stream,sizeof(gHeader2));

								unsigned short numberOfMembers = excludedMembers.size();

								message.escrever(numberOfMembers);
														
								for(set<unsigned short>::iterator itE = excludedMembers.begin(); itE != excludedMembers.end(); itE++)
								{
									message.escrever(*itE);
								}

								for(set<unsigned short>::iterator itM = itG->second.membros.begin(); itM != itG->second.membros.end(); itM++)
								{
									if(*itM==_idPeer)//The super Peer is member, must update
									{
										itG->second.souMembro = true;
										atualizarGrupo = true;
									}
									else
									{
										//send message
										std::map<unsigned short,Peer>::iterator itP = _peers.find(*itM);
										if(itP != _peers.end())
										{
											SDLNet_TCP_Send(itP->second.tcpSock,message.getDados(), message.getTamanhoDosDados()+1);
										}
										else
										{
											//TOTO: <ERROR> Peer not found
										}
									}
								}
							}
							else
							{
								groupDeleted = true;
								if(itG->second.souMembro)
								{
									//update Exclude										
									Grupo g;
									g.id = gHeader.header.groupId;
									g.souMembro = false;
									if(_fAtualizarGrupo)
									{
										_fAtualizarGrupo(g);
									}
								}

								_grupos.erase(itG);
							}
							//Send Message to Excluded Players	
							message.init();
							uMessageHeader header2;
							header2.header.info.version=1;
							header2.header.info.type=MessageType_CONTROL;
							header2.header.info.extension=0;
							header2.header.info.AddresseeType=AddresseeType_GROUP;
							header2.header.info.flags=FLAG_GRUPO;
							header2.header.addresseeID=gHeader.header.groupId;
							header2.header.senderID=_idPeer;
							message.escreverBytes(header2.stream,sizeof(header2));

							uGroupMessageHeader gHeader2;
							gHeader2.header.flags = Flag_Group_RESPONSE;
							gHeader2.header.type = GroupMessageType_Remove;
							gHeader2.header.groupId = gHeader.header.groupId;
							message.escreverBytes(gHeader2.stream,sizeof(gHeader2));

							for(set<unsigned short>::iterator itE = excludedMembers.begin(); itE != excludedMembers.end(); itE++)
							{
								if(*itE==_idPeer)//The super Peer is member, must update
								{
									if(!groupDeleted)
									{
										itG->second.souMembro = false;
										
										Grupo g;
										g.id = gHeader.header.groupId;
										g.souMembro = false;
										if(_fAtualizarGrupo)
										{
											_fAtualizarGrupo(g);
										}
									}
								}
								else
								{
									//send message
									std::map<unsigned short,Peer>::iterator itP = _peers.find(*itE);
									if(itP != _peers.end())
									{
										SDLNet_TCP_Send(itP->second.tcpSock,message.getDados(), message.getTamanhoDosDados()+1);
									}
									else
									{
										//TOTO: <ERROR> Peer not found
									}
								}
							}
						}
					}
				}
#pragma endregion							
#pragma region SuperPeer_RequestGroupCreate
				else if(gHeader.header.type == GroupMessageType_Create)
				{
					if(itG == _grupos.end())
					{
						Grupo g;
						
						g.id = gHeader.header.groupId;	
						g.lider = header.senderID;
						g.membros.insert(g.lider);
						g.senha = 0;
						g.usarSenha = false;
						g.souMembro = false;
						/*TODO Password*/
						unsigned short numberOfMembers = 0;

						if(checkFlag(gHeader.header.flags,Flag_Group_PEERS))
						{
							message.ler(numberOfMembers);

							if(numberOfMembers > 0)
							{				
								for(int i = 0; i < numberOfMembers; i++)
								{
									unsigned short membroId = 0;
									message.ler(membroId);
									g.membros.insert(membroId);
								}
							}
						}
						
						message.init();
						uMessageHeader header2;
						header2.header.info.version=1;
						header2.header.info.type=MessageType_CONTROL;
						header2.header.info.extension=0;
						header2.header.info.AddresseeType=AddresseeType_GROUP;
						header2.header.info.flags=FLAG_GRUPO;
						header2.header.addresseeID=g.id;
						header2.header.senderID=_idPeer;

						message.escreverBytes(header2.stream,sizeof(header2));

						uGroupMessageHeader gHeader2;
						gHeader2.header.flags = Flag_Group_PEERS | Flag_Group_RESPONSE;
						gHeader2.header.type = GroupMessageType_Create;
						gHeader2.header.groupId = g.id;

						message.escreverBytes(gHeader2.stream,sizeof(gHeader2));
						numberOfMembers = g.membros.size();
						message.escrever(numberOfMembers);
						for(set<unsigned short>::iterator itM = g.membros.begin(); itM != g.membros.end(); itM++)
						{
							message.escrever(*itM);
						}

						for(set<unsigned short>::iterator itM = g.membros.begin(); itM != g.membros.end(); itM++)
						{
							if(*itM==_idPeer)//The super Peer is member, must update
							{
								g.souMembro = true;
								atualizarGrupo = true;
							}
							else
							{
								//send message
								std::map<unsigned short,Peer>::iterator itP = _peers.find(*itM);
								if(itP != _peers.end())
								{
									SDLNet_TCP_Send(itP->second.tcpSock,message.getDados(), message.getTamanhoDosDados()+1);
								}
								else
								{
									//TOTO: <ERROR> Peer not found
								}
							}
						}

						_grupos.insert(std::pair<unsigned short, Grupo>(g.id,g));
						itG = _grupos.find(g.id);
					}
				}
#pragma endregion							
			}							
#pragma endregion
			
			if(_fAtualizarGrupo && atualizarGrupo)
			{
				_fAtualizarGrupo(itG->second);
			}
		}

	}
}

void ControleDeRede::receberConexoes(){
		//#################################################### CHECK NEW CONNECTIONS ########################################################################
	
	if (SDLNet_SocketReady(_listener))
    {
        // If we have room for more connection requets...
		if (_conexoesRecebidas.size() < CONNECTION_SLOTS/2)//(peers.size() < maxPeers)
        { 
			Peer tempPeer;
			tempPeer.tcpSock = SDLNet_TCP_Accept(_listener);
			tempPeer.tcpAddress.host = SDLNet_TCP_GetPeerAddress(tempPeer.tcpSock)->host;
			tempPeer.tcpAddress.port = 0;
			tempPeer.udpAddressIn.host = tempPeer.tcpAddress.host;
			tempPeer.udpAddressIn.port = 0;
			tempPeer.udpAddressOut.host = tempPeer.tcpAddress.host;
			tempPeer.udpAddressOut.port = 0;
			tempPeer.udpChannel = 0;
			tempPeer.peerID = 0;
			tempPeer.isRoot = false;
			tempPeer.isSuperPeer = false;

			SDLNet_TCP_AddSocket(_socketSet, tempPeer.tcpSock);

 
			buffer.init();
			uMessageHeader header;
			header.header.info.version=1;
			header.header.info.type=MessageType_CONNECTION;
			header.header.info.extension=0;
			header.header.info.AddresseeType=AddresseeType_PEER;
			header.header.info.flags=Flag_RESPONSE;
			header.header.addresseeID=0;
			header.header.senderID=0;
			buffer.escreverBytes(header.stream,sizeof(header));
			SDLNet_TCP_Send(tempPeer.tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);		

			_conexoesRecebidas.push_back(tempPeer);
																
#ifdef DEBUGFILE
ofstream debugFile;
debugFile.open(DEBUGLOGFILENAME,std::ios::app);
if(debugFile.is_open())
{
	debugFile << (_souSuperPeer?"Super ":"") << "Peer [" << (_souSuperPeer?_idSuperPeer:_idPeer) <<"] - New Incoming Connection (" << _conexoesRecebidas.size() << ")" << endl;
	debugFile.close();
}
#endif
        }
        else // If we reached our maximum requests capacity
        { 

            //// Accept the client connection to clear it from the incoming connections list
			TCPsocket tempSock = SDLNet_TCP_Accept(_listener);
 
			buffer.init();
			uMessageHeader header;
			header.header.info.version=1;
			header.header.info.type=MessageType_CONNECTION;
			header.header.info.extension=0;
			header.header.info.AddresseeType=AddresseeType_PEER;
			header.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
			header.header.addresseeID=0;
			header.header.senderID=0;
			buffer.escreverBytes(header.stream,sizeof(header));
			char FULLSIGNAL = '\f';
			buffer.escrever(FULLSIGNAL);

			SDLNet_TCP_Send(tempSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
 
            //// Shutdown, disconnect, and close the socket to the client
            SDLNet_TCP_Close(tempSock);
        }
 
    } // End of if server socket is has activity check
		
}

Peer* ControleDeRede::encontrarSuperPeerDisponivel()
{
	for(map<unsigned short, Peer>::iterator it = _superPeers.begin(); it != _superPeers.end(); it++)
	{
		if(it->second.numberOfConnections < _maxPeer)//TODO: check if is acceptingConnections
		{
			return &(it->second);
		}
	}
	return NULL;
}

void ControleDeRede::adminRemovePeerFromAllGroups(unsigned short pID)
{
	for(std::map<unsigned short, Grupo>::iterator itG = _grupos.begin(); itG !=_grupos.end();)
	{
		set<unsigned short>::iterator itD = itG->second.membros.find(pID);
		if(itD != itG->second.membros.end())
		{

			itG->second.membros.erase(itD);

			if(itG->second.lider == pID)
			{
				//exclude group
				buffer.init();
				uMessageHeader header2;
				header2.header.info.version=1;
				header2.header.info.type=MessageType_CONTROL;
				header2.header.info.extension=0;
				header2.header.info.AddresseeType=AddresseeType_GROUP;
				header2.header.info.flags=FLAG_GRUPO;
				header2.header.addresseeID=itG->second.id;
				header2.header.senderID=_idPeer;
				buffer.escreverBytes(header2.stream,sizeof(header2));

				uGroupMessageHeader gHeader2;
				gHeader2.header.flags = Flag_Group_RESPONSE;
				gHeader2.header.type = GroupMessageType_Remove;
				gHeader2.header.groupId = itG->second.id;
				buffer.escreverBytes(gHeader2.stream,sizeof(gHeader2));

				for(set<unsigned short>::iterator itM = itG->second.membros.begin(); itM != itG->second.membros.end(); itM++)
				{
					if(*itM==_idPeer)//The super Peer is member, must update
					{						
						Grupo g;
						g.id = itG->second.id;
						g.souMembro = false;
						_fAtualizarGrupo(g);
					}
					else
					{
						//send message
						std::map<unsigned short,Peer>::iterator itP = _peers.find(*itM);
						if(itP != _peers.end())
						{
							SDLNet_TCP_Send(itP->second.tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						}
						else
						{
							//TOTO: <ERROR> Peer not found
						}
					}
				}

				map<unsigned short, Grupo>::iterator itErase = itG;
				itG++;
				_grupos.erase(itErase);
				continue;
			}
			else
			{
				//exclude peer
				buffer.init();
				uMessageHeader header2;
				header2.header.info.version=1;
				header2.header.info.type=MessageType_CONTROL;
				header2.header.info.extension=0;
				header2.header.info.AddresseeType=AddresseeType_GROUP;
				header2.header.info.flags=FLAG_GRUPO;
				header2.header.addresseeID=itG->second.id;
				header2.header.senderID=_idPeer;

				buffer.escreverBytes(header2.stream,sizeof(header2));

				uGroupMessageHeader gHeader2;
				gHeader2.header.flags = Flag_Group_PEERS | Flag_Group_RESPONSE;
				gHeader2.header.type = GroupMessageType_Remove;
				gHeader2.header.groupId = itG->second.id;

				buffer.escreverBytes(gHeader2.stream,sizeof(gHeader2));

				unsigned short numberOfMembers = 1;

				buffer.escrever(numberOfMembers);
													
				buffer.escrever(pID);

				for(set<unsigned short>::iterator itM = itG->second.membros.begin(); itM != itG->second.membros.end(); itM++)
				{
					if(*itM==_idPeer)//The super Peer is member, must update
					{
						itG->second.souMembro = true;
						_fAtualizarGrupo(itG->second);

					}
					else
					{
						//send message
						std::map<unsigned short,Peer>::iterator itP = _peers.find(*itM);
						if(itP != _peers.end())
						{
							SDLNet_TCP_Send(itP->second.tcpSock,buffer.getDados(), buffer.getTamanhoDosDados()+1);
						}
						else
						{
							//TOTO: <ERROR> Peer not found
						}
					}
				}
			}

		}

		itG++;
	}
}

void ControleDeRede::tratarMensagens(){
	
#pragma region Incoming_Connections
	{
		std::list<Peer>::iterator it=_conexoesRecebidas.begin();
		while (it != _conexoesRecebidas.end())
		{
			if(SDLNet_SocketReady(it->tcpSock))
			{
				buffer.init();
				if(buffer.carregar(it->tcpSock))
				{
					uMessageHeader header;
					if(buffer.lerBytes(header.stream,sizeof(header)))
					{
#pragma region ConnectionMessages
						//is a Connection Message
						if(header.header.info.type == MessageType_CONNECTION)
						{
							//is a Request
							if(!checkFlag(header.header.info.flags,Flag_RESPONSE))
							{
#pragma region First_Request
								//is the 1st Request (must send Settings+Info)
								if(checkFlag(header.header.info.flags,Flag_1stREQUEST))
								{
									//Must Generate User Info & Settings	

									//Read TCP & UDP Ports
									buffer.ler(it->tcpAddress.port);
									buffer.ler(it->udpAddressIn.port);
									buffer.ler(it->udpAddressOut.port);
									IPaddress *incomingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
									if(incomingAddress)
									{
										it->tcpAddress.host = incomingAddress->host;
										it->udpAddressIn.host = incomingAddress->host;
										it->udpAddressOut.host = incomingAddress->host;
									}

#pragma region First_Request_Force_SuperPeer
									//Request for SuperPeer's Network 
									if(checkFlag(header.header.info.flags,Flag_SUPERPEER))
									{
										if((_superPeers.size()<_maxSuperPeer))
										{
											buffer.init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_CONNECTION;
											header2.header.info.extension=0;
																																		
											if(_souRoot)
											{
												_ultimoSuperPeerID++;
												header2.header.info.flags=Flag_RESPONSE | Flag_ACCEPT | Flag_SUPERPEER | Flag_1stREQUEST;
												header2.header.info.AddresseeType=AddresseeType_SUPERPEER;
												header2.header.addresseeID=_ultimoSuperPeerID;
												header2.header.senderID=_idSuperPeer;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												buffer.escrever(_chaveRedeSuperPeer);
												buffer.escrever(_tcpPortaN);
												buffer.escrever(_udpInPortaN);
												buffer.escrever(_udpOutPortaN);
												//write super peer info
												unsigned short numberOfSuperPeers = _superPeers.size();
												buffer.escrever(numberOfSuperPeers);
												std::map<unsigned short,Peer>::iterator itSp=_superPeers.begin();
												while ( itSp != _superPeers.end())
												{
													buffer.escrever(itSp->second.tcpAddress.host);
													buffer.escrever(itSp->second.tcpAddress.port);
													buffer.escrever(itSp->second.udpAddressIn.host);
													buffer.escrever(itSp->second.udpAddressIn.port);
													itSp++;
												}
												
												it->isSuperPeer=true;
												it->peerID=_ultimoSuperPeerID;
												//Add SuperPeer
												_superPeers.insert(std::pair<unsigned short,Peer>(_ultimoSuperPeerID,*it));

												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
												
												//Erase from List
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
											
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Root - Accept New Super Peer [" << _ultimoSuperPeerID << "]" << endl;
		debugFile.close();
	}
#endif
												continue;
					

											}
											else
											{
												//Redirect the request to the Root User
												header2.header.info.flags=Flag_RESPONSE | Flag_REDIRECT | Flag_SUPERPEER;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.addresseeID=0;
												header2.header.senderID=0;
												buffer.escreverBytes(header2.stream,sizeof(header2));

												//write redirect info
												if(_rootPeer)
												{						
													buffer.escrever(_rootPeer->tcpAddress.host);
													buffer.escrever(_rootPeer->tcpAddress.port);
													buffer.escrever(_rootPeer->udpAddressIn.host);
													buffer.escrever(_rootPeer->udpAddressIn.port);
												}
												else if(_superPeers.size() > 0)
												{
													buffer.escrever(_superPeers.begin()->second.tcpAddress.host);
													buffer.escrever(_superPeers.begin()->second.tcpAddress.port);
													buffer.escrever(_superPeers.begin()->second.udpAddressIn.host);
													buffer.escrever(_superPeers.begin()->second.udpAddressIn.port);
												}

												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

												SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
												//close connection
												SDLNet_TCP_Close(it->tcpSock);
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
												continue;
											}
										}
										else
										{
											buffer.init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_CONNECTION;
											header2.header.info.extension=0;
											header2.header.info.AddresseeType=AddresseeType_PEER;
											header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
											header2.header.addresseeID=0;
											header2.header.senderID=0;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											char FULLSIGNAL = '\f';
											buffer.escrever(FULLSIGNAL);
											
											SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

											SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
											//close connection
											SDLNet_TCP_Close(it->tcpSock);
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesRecebidas.erase(itErase);
											continue;
										}
									}								
#pragma endregion
#pragma region First_Request_Normal
									//Normal Request
									else
									{
										//check what to do with the player  (Accept || Redirect || StartSuperPeer)

										//Accept?
										if((_peers.size() < _maxPeer) && !_souRoot)
										{
											_ultimoPeerID++;
											
											it->peerID=_ultimoPeerID;
											it->udpChannel = it->peerID;
											SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
											SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));

											buffer.init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_CONNECTION;
											header2.header.info.extension=0;
											header2.header.info.AddresseeType=AddresseeType_PEER;
											header2.header.info.flags=Flag_RESPONSE | Flag_ACCEPT | Flag_1stREQUEST;
											header2.header.addresseeID=_ultimoPeerID;
											header2.header.senderID=_idSuperPeer;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											buffer.escrever(_chaveSalaDeJogo);
											buffer.escrever(_tcpPortaN);
											buffer.escrever(_udpInPortaN);
											buffer.escrever(_udpOutPortaN);
											//write Peer Info
											unsigned short numberOfPeers = _peers.size();
											buffer.escrever(numberOfPeers);
											std::map<unsigned short,Peer>::iterator itSp=_peers.begin();
											while ( itSp != _peers.end())
											{
												
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Add address to buffer [" << printIp(itSp->second.tcpAddress) << "] Total Peers = " << _peers.size() << endl;
		debugFile.close();
	}
#endif
												buffer.escrever(itSp->second.tcpAddress.host);
												buffer.escrever(itSp->second.tcpAddress.port);
												buffer.escrever(itSp->second.udpAddressIn.host);
												buffer.escrever(itSp->second.udpAddressIn.port);
												itSp++;
											}
											

											//Add Peer
											_peers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));
											
											SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
											
											SDL_Delay(500);

											instanciarTodosObjetosParaPeer(&(*it));
																
											if(_fSignalNovoJogador)
											{
												_fSignalNovoJogador(it->peerID);
											}

											//Erase from List
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesRecebidas.erase(itErase);

											_updateSuperPeerInfo = true;


#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Super Peer [" << _idSuperPeer <<"] - Accept New Peer [" << _ultimoPeerID << "] Total Peers = " << _peers.size() << endl;
		debugFile.close();
	}
#endif
											continue;
										}
										else
										{
											//Redirect? 
											Peer *sPeer = encontrarSuperPeerDisponivel(); 
											if(sPeer!=NULL) //check SuperPeer Network State for available rooms
											{
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_REDIRECT;
												header2.header.addresseeID=0;
												header2.header.senderID=0;
												buffer.escreverBytes(header2.stream,sizeof(header2));

												//write Redirect Info					
												buffer.escrever(sPeer->tcpAddress.host);
												buffer.escrever(sPeer->tcpAddress.port);
												buffer.escrever(sPeer->udpAddressIn.host);
												buffer.escrever(sPeer->udpAddressIn.port);

												sPeer->numberOfConnections++;

												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

												SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
												//close connection
												SDLNet_TCP_Close(it->tcpSock);
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
												//_updateSuperPeerInfo = true;
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Super Peer [" << _idSuperPeer <<"] - Redirect Peer [" << sPeer->peerID << "]" << endl;
		debugFile.close();
	}
#endif
												continue;
											}
											//Start New?
											else if(_superPeers.size()<_maxSuperPeer)
											{
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;

												if(_souRoot)
												{
													_ultimoSuperPeerID++;
													header2.header.info.flags=Flag_RESPONSE | Flag_ACCEPT | Flag_SUPERPEER | Flag_1stREQUEST;
													header2.header.info.AddresseeType=AddresseeType_SUPERPEER;
													header2.header.addresseeID=_ultimoSuperPeerID;
													header2.header.senderID=_idSuperPeer;
													buffer.escreverBytes(header2.stream,sizeof(header2));
													buffer.escrever(_chaveRedeSuperPeer);
													buffer.escrever(_tcpPortaN);
													buffer.escrever(_udpInPortaN);
													buffer.escrever(_udpOutPortaN);
													//write super peer info
													unsigned short numberOfSuperPeers = _superPeers.size();
													buffer.escrever(numberOfSuperPeers);
													std::map<unsigned short,Peer>::iterator itSp=_superPeers.begin();
													while ( itSp != _superPeers.end())
													{
														buffer.escrever(itSp->second.tcpAddress.host);
														buffer.escrever(itSp->second.tcpAddress.port);
														buffer.escrever(itSp->second.udpAddressIn.host);
														buffer.escrever(itSp->second.udpAddressIn.port);
														itSp++;
													}
												
													SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
												
													it->isSuperPeer=true;
													it->peerID=_ultimoSuperPeerID;
													//Add SuperPeer
													_superPeers.insert(std::pair<unsigned short,Peer>(_ultimoSuperPeerID,*it));
													//Erase from List
													std::list<Peer>::iterator itErase = it;
													it++;
													_conexoesRecebidas.erase(itErase);
											
	#ifdef DEBUGFILE
		ofstream debugFile;
		debugFile.open(DEBUGLOGFILENAME,std::ios::app);
		if(debugFile.is_open())
		{
			debugFile << "Root - Accept New Super Peer [" << _ultimoSuperPeerID << "]" << endl;
			debugFile.close();
		}
	#endif
													continue;
					

												}
												else
												{
													//Redirect the request to the Root User
													header2.header.info.flags=Flag_RESPONSE | Flag_REDIRECT | Flag_SUPERPEER;
													header2.header.info.AddresseeType=AddresseeType_PEER;
													header2.header.addresseeID=0;
													header2.header.senderID=0;
													buffer.escreverBytes(header2.stream,sizeof(header2));

													//write redirect info
													if(_rootPeer)
													{						
														buffer.escrever(_rootPeer->tcpAddress.host);
														buffer.escrever(_rootPeer->tcpAddress.port);
														buffer.escrever(_rootPeer->udpAddressIn.host);
														buffer.escrever(_rootPeer->udpAddressIn.port);
													}
													else if(_superPeers.size() > 0)
													{
														buffer.escrever(_superPeers.begin()->second.tcpAddress.host);
														buffer.escrever(_superPeers.begin()->second.tcpAddress.port);
														buffer.escrever(_superPeers.begin()->second.udpAddressIn.host);
														buffer.escrever(_superPeers.begin()->second.udpAddressIn.port);
													}

													SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

													SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
													//close connection
													SDLNet_TCP_Close(it->tcpSock);
													std::list<Peer>::iterator itErase = it;
													it++;
													_conexoesRecebidas.erase(itErase);
													_updateSuperPeerInfo = true;
													continue;
												}
												
											} 
											//Refuse
											else
											{
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
												header2.header.addresseeID=0;
												header2.header.senderID=0;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												char FULLSIGNAL = '\f';
												buffer.escrever(FULLSIGNAL);
												
												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
												
												SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
												//close connection
												SDLNet_TCP_Close(it->tcpSock);
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
												_updateSuperPeerInfo = true;
												continue;
											}
										}
									}				
#pragma endregion
								}								
#pragma endregion
#pragma region Default_Request
								else
								{
									//Load Peer Connection Info
									unsigned short connectionKey;
									buffer.ler<unsigned short>(connectionKey);
									buffer.ler(it->tcpAddress.port);
									buffer.ler(it->udpAddressIn.port);
									buffer.ler(it->udpAddressOut.port);
									IPaddress *incomingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
									if(incomingAddress)
									{
										it->tcpAddress.host = incomingAddress->host;
										it->udpAddressIn.host = incomingAddress->host;
										it->udpAddressOut.host = incomingAddress->host;
									}
									
#pragma region SuperPeer_Request
									//Request for SuperPeer's Network 
									if(checkFlag(header.header.info.flags,Flag_SUPERPEER))
									{
										if(connectionKey == _chaveRedeSuperPeer)
										{
											if(_superPeers.size()<_maxSuperPeer)
											{

												it->isSuperPeer=true;
												it->peerID=header.header.senderID;
												it->udpChannel = it->peerID;
												//TODO:
												//SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
												//SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));

												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_SUPERPEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_ACCEPT | Flag_SUPERPEER;
												header2.header.addresseeID=header.header.senderID;
												header2.header.senderID=_idSuperPeer;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												buffer.escrever(_tcpPortaN);
												buffer.escrever(_udpInPortaN);
												buffer.escrever(_udpOutPortaN);

												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
												
												//Add SuperPeer
												_superPeers.insert(std::pair<unsigned short,Peer>(it->peerID,*it));
												//Erase from List
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
#ifdef DEBUGFILE
ofstream debugFile;
debugFile.open(DEBUGLOGFILENAME,std::ios::app);
if(debugFile.is_open())
{
	debugFile << "Super Peer [" << _idSuperPeer <<"] - Accept Super Peer [" << header.header.senderID << "] <Key=" << connectionKey << ">"  << endl;
	debugFile.close();
}
#endif
												continue;
											} 
											//Refuse
											else
											{
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
												header2.header.addresseeID=0;
												header2.header.senderID=0;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												char FULLSIGNAL = '\f';
												buffer.escrever(FULLSIGNAL);
											
												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

												SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
												//close connection
												SDLNet_TCP_Close(it->tcpSock);
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
												continue;
											}
										}
										//Refuse WrongPassword
										else
										{
											buffer.init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_CONNECTION;
											header2.header.info.extension=0;
											header2.header.info.AddresseeType=AddresseeType_SUPERPEER;
											header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
											header2.header.addresseeID=header.header.senderID;
											header2.header.senderID=_idSuperPeer;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											char WRONGPASSSIGNAL = '\p';
											buffer.escrever(WRONGPASSSIGNAL);

											SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

											SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
											//close connection
											SDLNet_TCP_Close(it->tcpSock);
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesRecebidas.erase(itErase);
																																		
#ifdef DEBUGFILE
ofstream debugFile;
debugFile.open(DEBUGLOGFILENAME,std::ios::app);
if(debugFile.is_open())
{
	debugFile << "Wrong Password for Super Peer Connection - " << _chaveRedeSuperPeer << "!=" << connectionKey << endl;
	debugFile.close();
}
#endif
											continue;
										}
									}
#pragma endregion
									
#pragma region Peer_Request
									//Normal Request
									else
									{
										if(_chaveSalaDeJogo == connectionKey)
										{
											if((_peers.size() < _maxPeer) && !_souRoot)
											{
												it->peerID = header.header.senderID;
												it->udpChannel = it->peerID;
												SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
												SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));
												
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_ACCEPT;
												header2.header.addresseeID=header.header.senderID;
												header2.header.senderID=_idPeer;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												buffer.escrever(_tcpPortaN);
												buffer.escrever(_udpInPortaN);
												buffer.escrever(_udpOutPortaN);
												//Add Peer
												_peers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));

												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
											
												SDL_Delay(500);

												instanciarTodosObjetosParaPeer(&(*it));

												if(_fSignalNovoJogador)
												{
													_fSignalNovoJogador(it->peerID);
												}

												//Erase from List
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);

#ifdef DEBUGFILE
ofstream debugFile;
debugFile.open(DEBUGLOGFILENAME,std::ios::app);
if(debugFile.is_open())
{
	debugFile << "Peer [" << _idPeer <<"] - Accept Peer [" << header.header.senderID << "] <Key=" << connectionKey << ">" << endl;
	debugFile.close();
}
#endif
												continue;
											}
											//Refuse
											else
											{
												buffer.init();
												uMessageHeader header2;
												header2.header.info.version=1;
												header2.header.info.type=MessageType_CONNECTION;
												header2.header.info.extension=0;
												header2.header.info.AddresseeType=AddresseeType_PEER;
												header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
												header2.header.addresseeID=0;
												header2.header.senderID=0;
												buffer.escreverBytes(header2.stream,sizeof(header2));
												char FULLSIGNAL = '\f';
												buffer.escrever(FULLSIGNAL);
											
												SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

												SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
												//close connection
												SDLNet_TCP_Close(it->tcpSock);
												std::list<Peer>::iterator itErase = it;
												it++;
												_conexoesRecebidas.erase(itErase);
												continue;
											}
										}
										//Refuse WrongPassword
										else
										{
											buffer.init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_CONNECTION;
											header2.header.info.extension=0;
											header2.header.info.AddresseeType=AddresseeType_PEER;
											header2.header.info.flags=Flag_RESPONSE | Flag_REFUSE;
											header2.header.addresseeID=header.header.senderID;
											header2.header.senderID=_idPeer;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											char WRONGPASSSIGNAL = '\p';
											buffer.escrever(WRONGPASSSIGNAL);

											SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);

											SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
											//close connection
											SDLNet_TCP_Close(it->tcpSock);
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesRecebidas.erase(itErase);
#ifdef DEBUGFILE
ofstream debugFile;
debugFile.open(DEBUGLOGFILENAME,std::ios::app);
if(debugFile.is_open())
{
	debugFile << "Wrong Password for Peer Connection - " << _chaveSalaDeJogo << "!=" << connectionKey << endl;
	debugFile.close();
}
#endif
										}
									}
#pragma endregion
								}	
#pragma endregion
							}
						}
						else
						{
#ifdef DEBUGFILE
							ofstream debugFile;
							debugFile.open(DEBUGLOGFILENAME,std::ios::app);
							if(debugFile.is_open())
							{
								string msg = "Incoming connection MessageType ERROR!";
								string messageType="";
								switch (header.header.info.type)
								{
								case MessageType_COMMON:
									messageType=" <MessageType_COMMON>";
									break;
								case MessageType_CONTROL:
									messageType=" <MessageType_CONTROL>";
									break;
								case MessageType_OBJECT:
									messageType=" <MessageType_OBJECT>";
									break;
								default:
									break;
								}
								debugFile << "Incoming [" << it->peerID <<"] - Mensagem: " << msg << messageType << endl;
								debugFile.close();
							}
#endif
						}
#pragma endregion
					}
				}
				else // If we've received a 0 byte message from the server then we've lost the connection!
				{
					// ...generate a suitable error message...
#ifdef DEBUGFILE
					string msg = "Lost connection to the server!";
					ofstream debugFile;
					debugFile.open(DEBUGLOGFILENAME,std::ios::app);
					if(debugFile.is_open())
					{
						debugFile << "Outgoing [" << it->peerID <<"] - Mensagem: " << msg << endl;
						debugFile.close();
					}
#endif
					
					SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
					//close connection
					SDLNet_TCP_Close(it->tcpSock);
					std::list<Peer>::iterator itErase = it;
					it++;
					_conexoesRecebidas.erase(itErase);
					_updateSuperPeerInfo = true;
					continue;

					// ...and then throw it as an exception!
					//SocketException e(msg);
					//throw e;
				}
			}
			it++;
		}
	}
#pragma endregion
	
#pragma region Outgoing_Connections
	{
		std::list<Peer>::iterator it=_conexoesSolicitadas.begin();
		while (it != _conexoesSolicitadas.end())
		{
			/*TODO*/
			if(SDLNet_SocketReady(it->tcpSock))
			{
				buffer.init();
				if(buffer.carregar(it->tcpSock))
				{
					uMessageHeader header;
					if(buffer.lerBytes(header.stream,sizeof(header)))
					{
#pragma region ConnectionMessages
						//is a Connection Message
						if(header.header.info.type == MessageType_CONNECTION)
						{
							//is a Response
							if(checkFlag(header.header.info.flags,Flag_RESPONSE))
							{
								//is the 1st Response

#pragma region Response_ACK
								if(header.header.info.flags == Flag_RESPONSE)
								{
#pragma region Send_First_Request
									//is the first connection (must ask for Settings+Info)
									if(!_primeiraConexao)
									{
										buffer.init();
										uMessageHeader header2;
										header2.header.info.version=1;
										header2.header.info.type=MessageType_CONNECTION;
										header2.header.info.extension=0;
										header2.header.info.AddresseeType=AddresseeType_PEER;

										if(it->isSuperPeer)
										{
											header2.header.info.flags=Flag_1stREQUEST | Flag_SUPERPEER;
										}
										else
										{
											header2.header.info.flags=Flag_1stREQUEST;
										}

										header2.header.addresseeID=0;
										header2.header.senderID=0;
										buffer.escreverBytes(header2.stream,sizeof(header2));
										buffer.escrever(_tcpPortaN);
										buffer.escrever(_udpInPortaN);
										buffer.escrever(_udpOutPortaN);

										SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
											
																
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "First Connection Request Sent " << endl;
		debugFile.close();
	}
#endif
									}
#pragma endregion
#pragma region Send_Connection_Request
									//is a normal connection (must send Peer/User Info)
									else
									{
										buffer.init();
										uMessageHeader header2;
										header2.header.info.version=1;
										header2.header.info.type=MessageType_CONNECTION;
										header2.header.info.extension=0;
										
#pragma region Connection_Request_SuperPeer
										if(_souSuperPeer && it->isSuperPeer)
										{
											header2.header.info.AddresseeType=AddresseeType_SUPERPEER;
											header2.header.info.flags=Flag_SUPERPEER;
											header2.header.addresseeID=it->peerID;
											header2.header.senderID=_idSuperPeer;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											//write Peer Info
											buffer.escrever(_chaveRedeSuperPeer);
											buffer.escrever(_tcpPortaN);
											buffer.escrever(_udpInPortaN);
											buffer.escrever(_udpOutPortaN);											
																											
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Connect to SuperPeer with Key = " << _chaveRedeSuperPeer << endl;
		debugFile.close();
	}
#endif
										}
#pragma endregion
										
#pragma region Connection_Request_Peer
										else
										{
											header2.header.info.AddresseeType=AddresseeType_PEER;
											header2.header.info.flags=0;
											header2.header.addresseeID=it->peerID;
											header2.header.senderID=_idPeer;
											buffer.escreverBytes(header2.stream,sizeof(header2));
											//write Peer Info
											buffer.escrever(_chaveSalaDeJogo);	
											buffer.escrever(_tcpPortaN);
											buffer.escrever(_udpInPortaN);
											buffer.escrever(_udpOutPortaN);																							
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Connect to Peer with Key = " << _chaveSalaDeJogo << endl;
		debugFile.close();
	}
#endif
										}
#pragma endregion
										
										SDLNet_TCP_Send(it->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
									}
#pragma endregion
								}
#pragma endregion
#pragma region Connection_Refused
								//connection refused
								else if(checkFlag(header.header.info.flags,Flag_REFUSE))
								{
									SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
									//close connection
									SDLNet_TCP_Close(it->tcpSock);
									std::list<Peer>::iterator itErase = it;
									it++;
									_conexoesSolicitadas.erase(itErase);
									continue;
								}
#pragma endregion
#pragma region Connection_Accepted
								//connection accepted
								else if(checkFlag(header.header.info.flags,Flag_ACCEPT))
								{
#pragma region First_Request_Accepted
									//if is 1stConnection --> get Settings+Info
									if(checkFlag(header.header.info.flags,Flag_1stREQUEST))
									{
										_primeiraConexao=true;

#pragma region Start_SuperPeer
										//Start a Super Peer and connect to the SuperPeer's Network
										if(checkFlag(header.header.info.flags,Flag_SUPERPEER))
										{
											it->isRoot = true;
											it->isSuperPeer = true;
											it->peerID = header.header.senderID;
											_souSuperPeer = true;
											_souRoot = false;
											_idSuperPeer = header.header.addresseeID;
											_idPeer = 0;
											_ultimoPeerID = 0;

											buffer.ler(_chaveRedeSuperPeer);
											buffer.ler(it->tcpAddress.port);
											buffer.ler(it->udpAddressIn.port);
											buffer.ler(it->udpAddressOut.port);
											IPaddress *outgoingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
											if(outgoingAddress)
											{
												it->tcpAddress.host = outgoingAddress->host;
												it->udpAddressIn.host = outgoingAddress->host;
												it->udpAddressOut.host = outgoingAddress->host;
											}

											//TODO: bind UDP for Super Peer											
											//SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
											//SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));

											srand((unsigned)time(NULL));		

											for(int k=0;k<_idSuperPeer;k++)
											{
												rand();
											}

											_chaveSalaDeJogo = rand(); //TODO: change NetKey definition

																						
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Super Peer [" << _idSuperPeer <<"] - Connected to the Super Peer [" << it->peerID << "] <SP Key=" << _chaveRedeSuperPeer << "> <Room Key=" << _chaveSalaDeJogo << ">" << endl;
		debugFile.close();
	}
#endif							
											//Add SuperPeer
											_superPeers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));
											_rootPeer = &_superPeers[it->peerID];
											//Erase from List
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesSolicitadas.erase(itErase);
											
											//Connect to other super peers
											IPaddress tcp, udp;
											unsigned short size = 0;
											buffer.ler(size);
											for(int j=0; j<size;j++)
											{
												buffer.ler(tcp.host);
												buffer.ler(tcp.port);
												buffer.ler(udp.host);
												buffer.ler(udp.port);
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Conectar [" << printIp(tcp) <<"] <SP Key=" << _chaveRedeSuperPeer << ">" << endl;
		debugFile.close();
	}
#endif		
												conectar(tcp,true);
											}

											continue;
										}
#pragma endregion
#pragma region Start_Peer
										//Start a Peer and connect to the other users
										else
										{
											it->isRoot = false;
											it->isSuperPeer = true;
											it->peerID = 0; //The SuperPeer's User ID is equal to zero (0)
											
											_souSuperPeer = false;
											_souRoot = false;
											_idPeer = header.header.addresseeID;
											_idSuperPeer = header.header.senderID; //save the Master's Super Peer ID
											
											buffer.ler(_chaveSalaDeJogo);
											buffer.ler(it->tcpAddress.port);
											buffer.ler(it->udpAddressIn.port);
											buffer.ler(it->udpAddressOut.port);
											IPaddress *outgoingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
											if(outgoingAddress)
											{
												it->tcpAddress.host = outgoingAddress->host;
												it->udpAddressIn.host = outgoingAddress->host;
												it->udpAddressOut.host = outgoingAddress->host;
											}
											
											it->udpChannel = it->peerID;
											SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
											SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));

											//Add Peer
											_peers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));


#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Peer [" << _idPeer <<"] - Connected to the Super Peer [" << _idSuperPeer << "] with NetKey = " << _chaveSalaDeJogo << endl;
		debugFile.close();
	}
#endif
											//Connect to other peers
											IPaddress tcp, udp;
											unsigned short size = 0;
											buffer.ler(size);
											for(int j=0; j<size;j++)
											{
												buffer.ler(tcp.host);
												buffer.ler(tcp.port);
												buffer.ler(udp.host);
												buffer.ler(udp.port);
												conectar(tcp,false);
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Conectar [" << printIp(tcp) <<"] <Room Key=" << _chaveSalaDeJogo << ">" << endl;
		debugFile.close();
	}
#endif		
											}

											instanciarTodosObjetosParaPeer(&(_peers[it->peerID]));
											
											if(_fSignalNovoJogador)
											{
												_fSignalNovoJogador(it->peerID);
											}
	
											//Erase from List
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesSolicitadas.erase(itErase);

											continue;

											//buffer.Init();
											//uMessageHeader header2;
											//header2.header.info.version=1;
											//header2.header.info.type=MessageType_OBJECT;
											//header2.header.info.extension=0;
											//header2.header.info.AddresseeType=AddresseeType_USER;
											//header2.header.info.flags=0;
											//header2.header.addresseeID = outgoingPeers[i].peerID;
											//header2.header.senderID = userID;
											//buffer.writeBytes(header2.stream,sizeof(header2));

											//SDLNet_TCP_Send(outgoingPeers[i].tcpSock, (void *)buffer.getData(), buffer.getDataSize()+1);
											
										}
#pragma endregion
									}
#pragma endregion
#pragma region Connection_Request_Accepted
									else
									{
#pragma region Add_SuperPeer
										//Connect to SuperPeer's Network
										if(checkFlag(header.header.info.flags,Flag_SUPERPEER))
										{
											it->isRoot = false;
											it->isSuperPeer = true;
											it->peerID = header.header.senderID;
																						
											buffer.ler(it->tcpAddress.port);
											buffer.ler(it->udpAddressIn.port);
											buffer.ler(it->udpAddressOut.port);
											IPaddress *outgoingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
											if(outgoingAddress)
											{
												it->tcpAddress.host = outgoingAddress->host;
												it->udpAddressIn.host = outgoingAddress->host;
												it->udpAddressOut.host = outgoingAddress->host;
											}

											//TODO: Bind SuperPeer UDP Address to a SuperPeer UDP Socket
											//Add SuperPeer
											_superPeers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));
											//Erase from List
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesSolicitadas.erase(itErase);
											continue;
										}
#pragma endregion
#pragma region Add_Peer
										//Connect to Peer's Network
										else
										{
											it->isRoot = false;
											it->isSuperPeer = false;
											it->peerID = header.header.senderID;
											
											buffer.ler(it->tcpAddress.port);
											buffer.ler(it->udpAddressIn.port);
											buffer.ler(it->udpAddressOut.port);
											IPaddress *outgoingAddress = SDLNet_TCP_GetPeerAddress(it->tcpSock);
											if(outgoingAddress)
											{
												it->tcpAddress.host = outgoingAddress->host;
												it->udpAddressIn.host = outgoingAddress->host;
												it->udpAddressOut.host = outgoingAddress->host;
											}

											it->udpChannel = it->peerID;
											SDLNet_UDP_Bind(_udpIn,it->udpChannel,&(it->udpAddressOut));
											SDLNet_UDP_Bind(_udpOut,it->udpChannel,&(it->udpAddressIn));

#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "<Peer: " << it->peerID <<"> - Accepted the Connection" << endl;
		debugFile.close();
	}
#endif

											//Add Peer
											_peers.insert(std::pair<unsigned short,Peer>(it->peerID,(*it)));
											
											if(_fSignalNovoJogador)
											{
												_fSignalNovoJogador(it->peerID);
											}

											instanciarTodosObjetosParaPeer(&(*it));
											//Erase from List
											std::list<Peer>::iterator itErase = it;
											it++;
											_conexoesSolicitadas.erase(itErase);
											
											/*buffer.Init();
											uMessageHeader header2;
											header2.header.info.version=1;
											header2.header.info.type=MessageType_OBJECT;
											header2.header.info.extension=0;
											header2.header.info.AddresseeType=AddresseeType_USER;
											header2.header.info.flags=0;
											header2.header.addresseeID = outgoingPeers[i].peerID;
											header2.header.senderID = userID;
											buffer.writeBytes(header2.stream,sizeof(header2));

											SDLNet_TCP_Send(outgoingPeers[i].tcpSock, (void *)buffer.getData(), buffer.getDataSize()+1);*/

											continue;

										}
#pragma endregion
									}
#pragma endregion
								}
								
#pragma endregion
#pragma region Connection_Redirect
								//redirect the connection to annother Super Peer
								else if(checkFlag(header.header.info.flags,Flag_REDIRECT))
								{
									//if(checkFlag(header.header.info.flags,Flag_1stREQUEST))
									//{
									//	firstConnectionCompleted=true;
									//}

									IPaddress tcpRedirect, udpRedirect;

									//Load Redirect Info
									buffer.ler(tcpRedirect.host);
									buffer.ler(tcpRedirect.port);
									buffer.ler(udpRedirect.host);
									buffer.ler(udpRedirect.port);
																		
									SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
									//close connection
									SDLNet_TCP_Close(it->tcpSock);
									std::list<Peer>::iterator itErase = it;
									it++;
									_conexoesSolicitadas.erase(itErase);

									//Redirect Connection
									conectar(tcpRedirect,checkFlag(header.header.info.flags,Flag_SUPERPEER));
											
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Redirect Connection" << endl;
		debugFile.close();
	}
#endif
									continue;
								}
#pragma endregion
							}
						}
						else
						{
#ifdef DEBUGFILE
							ofstream debugFile;
							debugFile.open(DEBUGLOGFILENAME,std::ios::app);
							if(debugFile.is_open())
							{
								string msg = "Outgoing connection MessageType ERROR!";
								string messageType="";
								switch (header.header.info.type)
								{
								case MessageType_COMMON:
									messageType=" <MessageType_COMMON>";
									break;
								case MessageType_CONTROL:
									messageType=" <MessageType_CONTROL>";
									break;
								case MessageType_OBJECT:
									messageType=" <MessageType_OBJECT>";
									break;
								default:
									break;
								}
								debugFile << "Outgoing [" << it->peerID <<"] - Mensagem: " << msg << messageType << endl;
								debugFile.close();
							}
#endif
						}
#pragma endregion
					}
					else // If we've received a 0 byte message from the server then we've lost the connection!
					{
						// ...generate a suitable error message...
#ifdef DEBUGFILE
						string msg = "Lost connection to the server!";
						ofstream debugFile;
						debugFile.open(DEBUGLOGFILENAME,std::ios::app);
						if(debugFile.is_open())
						{
							debugFile << "Outgoing [" << it->peerID <<"] - Mensagem: " << msg << endl;
							debugFile.close();
						}
#endif					
						SDLNet_TCP_DelSocket(_socketSet,it->tcpSock);
						//close connection
						SDLNet_TCP_Close(it->tcpSock);
						std::list<Peer>::iterator itErase = it;
						it++;
						_conexoesSolicitadas.erase(itErase);
						continue;

						// ...and then throw it as an exception!
						//SocketException e(msg);
						//throw e;
					}
				}
			}
			it++;
		}
	}
#pragma endregion
	
#pragma region Peer_Messages
	{
		if(!_souRoot)
		{
			std::map<unsigned short,Peer>::iterator it=_peers.begin();
			while ( it != _peers.end())
			{
				/*TODO*/
				if(SDLNet_SocketReady(it->second.tcpSock))
				{
					buffer.init();
					//int serverResponseByteCount = SDLNet_TCP_Recv(peers[i].tcpSock, buffer, bufferSize);
					if (buffer.carregar(it->second.tcpSock))
					{
						uMessageHeader header;
						// Define a string with a blank message
						if(buffer.lerBytes(header.stream,sizeof(header)))
						{
#pragma region Object_Messages
							if(header.header.info.type == MessageType_OBJECT)
							{
#pragma region Instantiate_Object
								if(checkFlag(header.header.info.flags,Flag_InstantiateNetworkObject))
								{
									if(_fCriarObjeto)
									{
										unsigned short objType = 0;
										unsigned short objId = 0;
										buffer.ler(objType);
										buffer.ler(objId);
										std::map<unsigned short,ObjetoDeRede*>::iterator itO = it->second.objetos.find(objId);

										if(itO == it->second.objetos.end())
										{
											ObjetoDeRede::NetworkInstantiate = true;
											ObjetoDeRede::NetworkInstantiateId = objId;
											ObjetoDeRede* obj = _fCriarObjeto(objType,_dadosCriarObjeto);
											it->second.adicionarObjeto(obj);
											ObjetoDeRede::NetworkInstantiate = false;
											obj->deserializarNovo(&buffer);
#ifdef DEBUGFILE
											ofstream debugFile;
											debugFile.open(DEBUGLOGFILENAME,std::ios::app);
											if(debugFile.is_open())
											{
												debugFile << "[TCP] <Peer:" << header.header.senderID << ">Instantiating New Network Object <" << objId << ">" << endl;
												debugFile.close();
											}
#endif
										}
										else
										{	
#ifdef DEBUGFILE
											ofstream debugFile;
											debugFile.open(DEBUGLOGFILENAME,std::ios::app);
											if(debugFile.is_open())
											{
												debugFile << "[TCP] <Peer:" << header.header.senderID << "> Instantiating New Network Object <" << objId << "> [FAIL: Object already exist]" << endl;
												debugFile.close();
											}
#endif
										}
									}
								}
#pragma endregion
#pragma region Update_Object
								else if(checkFlag(header.header.info.flags,Flag_UpdateNetworkObject))
								{
									/*TODO*/
									unsigned short objId = 0;
									buffer.ler(objId);
									
									std::map<unsigned short,ObjetoDeRede*>::iterator itO = it->second.objetos.find(objId);

									if(itO != it->second.objetos.end())
									{
										itO->second->deserializar(&buffer);
									}
									else 
									{
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "[TCP] <Peer:" << header.header.senderID << "> Object Not Found <" << objId << "> - Sending Request" << endl;
		debugFile.close();
	}
#endif
	
										/*TODO - timer/timeout*/
										buffer.init();
										uMessageHeader header2;
										header2.header.info.version = 1;
										header2.header.info.extension=0;
										header2.header.info.type = MessageType_OBJECT;
										header2.header.info.AddresseeType = AddresseeType_PEER;
										header2.header.addresseeID = header.header.senderID; //Group 0 = all connected peers of the 2nd Layer Network
										header2.header.senderID = _idPeer;
										header2.header.info.flags=Flag_RequestNetworkObject;

										buffer.escreverBytes(header2.stream, sizeof(header2));
										buffer.escrever<unsigned short>(objId);

										SDLNet_TCP_Send(it->second.tcpSock,buffer.getDados(),buffer.getTamanhoDosDados()+1);
									}
								}
#pragma endregion
#pragma region Request_Object
								else if(checkFlag(header.header.info.flags,Flag_RequestNetworkObject))
								{
									/*TODO*/
									unsigned short objId = 0;
									buffer.ler(objId);
									
									std::map<unsigned short,ObjetoDeRede*>::iterator itO = it->second.objetos.find(objId);

									if(itO != it->second.objetos.end())
									{
										enviarMensagemInstanciarObjeto(itO->second,&(it->second));
									}
									else 
									{
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "[TCP] <Peer:" << header.header.senderID << "> Requested Object Not Found <" << objId << "> - Sending Request" << endl;
		debugFile.close();
	}
#endif
									}
								}
#pragma endregion
							}
#pragma endregion
#pragma region Common_Messages
							else if(header.header.info.type == MessageType_COMMON)
							{
								if(_fTratarMensagensRecebidas)
								{
									_fTratarMensagensRecebidas(header.header,buffer);
								}
								else
								{
									/*TODO - Erro: função de recebimento de mensagens não foi vinculada ao controle de rede*/
								}
							}
#pragma endregion
#pragma region Control_Messages
							else if(header.header.info.type == MessageType_CONTROL)
							{
								/*TODO: Group & BUS*/
								if(checkFlag(header.header.info.flags,FLAG_GRUPO))
								{
									tratarMensagemControleGrupo(header.header,buffer);
								}
							}
#pragma endregion
						}
					}
#pragma region Lost_Connection
					else
					{
						//Lost Connection
						SDLNet_TCP_DelSocket(_socketSet,it->second.tcpSock);
						SDLNet_UDP_Unbind(_udpIn,it->second.udpChannel);
						SDLNet_UDP_Unbind(_udpOut,it->second.udpChannel);
						//close connection
						SDLNet_TCP_Close(it->second.tcpSock);
						std::map<unsigned short,Peer>::iterator itErase = it;
						it++;
						itErase->second.destruirObjetos();
						if(itErase->second.isSuperPeer)
						{
							/*TODO - Choose new admin*/
							_adminPeer = NULL;
						}

						if(_fSignalJogadorDesconectou)
						{
							_fSignalJogadorDesconectou(itErase->second.peerID);
						}

						adminRemovePeerFromAllGroups(itErase->second.peerID);

						_peers.erase(itErase);
						


						_updateSuperPeerInfo = true;
						continue;
					}
#pragma endregion
				}
				it++;
			}
		}
	}
#pragma endregion
	
#pragma region SuperPeer_Messages
	{
		if(_souSuperPeer)
		{
			std::map<unsigned short,Peer>::iterator it=_superPeers.begin();
			while ( it != _superPeers.end())
			{
				/*TODO*/
				//Receber message control - atualizar
				if(SDLNet_SocketReady(it->second.tcpSock))
				{
					buffer.init();
					if (buffer.carregar(it->second.tcpSock))
					{
						uMessageHeader header;
						if(buffer.lerBytes(header.stream,sizeof(header)))
						{
#pragma region Control_Messages
							if(header.header.info.type == MessageType_CONTROL)
							{
								/*TODO: BUS*/
#pragma region Update_SuperPeer_Info
								if(checkFlag(header.header.info.flags,FLAG_ATUALIZAR))
								{
									unsigned short connectionsCount = 0;
									buffer.ler<unsigned short>(connectionsCount);
									it->second.numberOfConnections = connectionsCount;
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "<SuperPeer " << it->second.peerID << "> - Update [" << it->second.numberOfConnections << "]" << endl;
		debugFile.close();
	}
#endif
								}
#pragma endregion
							}
#pragma endregion
						}
					}
#pragma region Lost_Connection
					else
					{
#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "Lost Connection to Super Peer [" << it->second.peerID << "]" << endl;
		debugFile.close();
	}
#endif
						//Lost Connection
						SDLNet_TCP_DelSocket(_socketSet,it->second.tcpSock);
						//SDLNet_UDP_Unbind(_udpIn,it->second.udpChannel);
						//SDLNet_UDP_Unbind(_udpOut,it->second.udpChannel);
						//close connection
						SDLNet_TCP_Close(it->second.tcpSock);
						std::map<unsigned short,Peer>::iterator itErase = it;
						it++;
						_superPeers.erase(itErase);
						continue;
					}
#pragma endregion
				}
				it++;
			}
		}
	}
#pragma endregion
	
#pragma region UDP_Messages
	{
		while(SDLNet_SocketReady(_udpIn))//TODO: code a breaker to avoid infinite loop
		{
			buffer.init();
			//int serverResponseByteCount = SDLNet_TCP_Recv(peers[i].tcpSock, buffer, bufferSize);
			if (buffer.carregar(_udpIn))
			{
				uMessageHeader header;
				// Define a string with a blank message
				if(buffer.lerBytes(header.stream,sizeof(header)))
				{
#pragma region Object_Messages
					if(header.header.info.type == MessageType_OBJECT)
					{
#pragma region Update_Object
						if(checkFlag(header.header.info.flags,Flag_UpdateNetworkObject))
						{
							unsigned short objId = 0;
							buffer.ler(objId);
							unsigned short peerId = header.header.senderID;

							std::map<unsigned short,Peer>::iterator it = _peers.find(peerId);
							
							if(it != _peers.end())
							{
								std::map<unsigned short,ObjetoDeRede*>::iterator itO = it->second.objetos.find(objId);

								if(itO != it->second.objetos.end())
								{
									itO->second->deserializar(&buffer);
								}
								else 
								{
#ifdef DEBUGFILE
									ofstream debugFile;
									debugFile.open(DEBUGLOGFILENAME,std::ios::app);
									if(debugFile.is_open())
									{
										debugFile << "[UDP] <Peer:" << header.header.senderID << "> Object Not Found <" << objId << "> - Sending Request" << endl;
										debugFile.close();
									}
#endif
									/*TODO - timer/timeout*/
									buffer.init();
									uMessageHeader header2;
									header2.header.info.version = 1;
									header2.header.info.extension=0;
									header2.header.info.type = MessageType_OBJECT;
									header2.header.info.AddresseeType = AddresseeType_PEER;
									header2.header.addresseeID = header.header.senderID; //Group 0 = all connected peers of the 2nd Layer Network
									header2.header.senderID = _idPeer;
									header2.header.info.flags=Flag_RequestNetworkObject;

									buffer.escreverBytes(header2.stream, sizeof(header2));
									buffer.escrever<unsigned short>(objId);

									SDLNet_TCP_Send(it->second.tcpSock,buffer.getDados(),buffer.getTamanhoDosDados()+1);
								}
							}
							else 
							{
#ifdef DEBUGFILE
								ofstream debugFile;
								debugFile.open(DEBUGLOGFILENAME,std::ios::app);
								if(debugFile.is_open())
								{
									debugFile << "[UDP] <Peer:" << header.header.senderID << "> Peer Not Found" << endl;
									debugFile.close();
								}
#endif	
							}
						}
#pragma endregion
					}
#pragma endregion
#pragma region Common_Messages
							else if(header.header.info.type == MessageType_COMMON)
							{
								if(_fTratarMensagensRecebidas)
								{
									_fTratarMensagensRecebidas(header.header,buffer);
								}
								else
								{
									/*TODO - Erro: função de recebimento de mensagens não foi vinculada ao controle de rede*/
								}
							}
#pragma endregion
#pragma region Control_Messages
					else if(header.header.info.type == MessageType_CONTROL)
					{
						/*TODO*/
					}
#pragma endregion
				}
			}
		}
	}
#pragma endregion
}

void ControleDeRede::atualizarObjetos(){
	//atualiza objetos de rede
	for (std::map<unsigned short,ObjetoDeRede*>::iterator it=_objetos.begin(); it!=_objetos.end(); ++it)
	{
		(*it).second->atualizar();
		if(it->second->checkSendTimer())
		{
			buffer.init();
			uMessageHeader header;
			header.header.info.version = 1;
			header.header.info.extension=0;
			header.header.info.type = MessageType_OBJECT;
			header.header.info.AddresseeType = AddresseeType_GROUP;
			header.header.addresseeID = 0; //Group 0 = all connected peers of the 2nd Layer Network
			header.header.senderID = _idPeer;
			header.header.info.flags=Flag_UpdateNetworkObject;

			buffer.escreverBytes(header.stream, sizeof(header));
			unsigned short objId = (*it).second->getID();
			buffer.escrever<unsigned short>(objId);
			(*it).second->serializar(&buffer);
		
			if(it->second->enviar)
			{
				if((*it).second->usarUDP)
				{
					UDPpacket *packet = SDLNet_AllocPacket(buffer.getTamanhoDoBuffer());
			
					memcpy(packet->data,buffer.getDados(),buffer.getTamanhoDosDados()+1);

					packet->len = buffer.getTamanhoDosDados()+1;

					for(std::map<unsigned short,Peer>::iterator itP=_peers.begin();itP!=_peers.end();itP++)
					{
						//send updates using UDP
						SDLNet_UDP_Send(_udpOut,itP->second.udpChannel,packet);
					}
					SDLNet_FreePacket(packet);
				}
				else
				{
					for(std::map<unsigned short,Peer>::iterator itP=_peers.begin();itP!=_peers.end();itP++)
					{
						SDLNet_TCP_Send(itP->second.tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
					}
				}
			}
		}
	}
}

void ControleDeRede::instanciarTodosObjetosParaPeer(Peer *peer)
{
	for (std::multimap<unsigned short,ObjetoDeRede*>::iterator it=_objetos.begin(); it!=_objetos.end(); ++it)
	{
		enviarMensagemInstanciarObjeto((*it).second,peer);
	}
}

void ControleDeRede::enviarMensagemInstanciarObjeto(ObjetoDeRede *obj, Peer *peer/*if NULL send to all connected peers*/)
{
	uMessageHeader header;
	header.header.info.version = 1;
	header.header.info.extension=0;
	header.header.info.type = MessageType_OBJECT;
	if(peer==NULL)
	{
		header.header.info.AddresseeType = AddresseeType_GROUP;
		header.header.addresseeID = 0; //Group 0 = all connected peers of the 2nd Layer Network
	}
	else
	{
		header.header.info.AddresseeType = AddresseeType_PEER;
		header.header.addresseeID = peer->peerID;
	}
	header.header.senderID = _idPeer;
	header.header.info.flags=Flag_InstantiateNetworkObject;

	buffer.init();
	buffer.escreverBytes(header.stream, sizeof(header));
	unsigned short objType = obj->getTipo();
	unsigned short objId = obj->getID();
	buffer.escrever<unsigned short>(objType);
	buffer.escrever<unsigned short>(objId);
	obj->serializarNovo(&buffer);
	
	if(peer==NULL)
	{

#ifdef DEBUGFILE
	ofstream debugFile;
	debugFile.open(DEBUGLOGFILENAME,std::ios::app);
	if(debugFile.is_open())
	{
		debugFile << "[TCP] Instantiating New Network Object<" << obj->getID() << ">" << endl;
		debugFile.close();
	}
#endif
		for(std::map<unsigned short,Peer>::iterator itP=_peers.begin();itP!=_peers.end();itP++)
		{
			SDLNet_TCP_Send(itP->second.tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
		}
	}
	else
	{		

		int len = SDLNet_TCP_Send(peer->tcpSock, (void *)buffer.getDados(), buffer.getTamanhoDosDados()+1);
#ifdef DEBUGFILE
		ofstream debugFile;
		debugFile.open(DEBUGLOGFILENAME,std::ios::app);
		if(debugFile.is_open())
		{
			debugFile << "[TCP] Instantiating Network Object <" << obj->getID() << "> for the New Peer <" << peer->peerID << "> Packet Size = " << len << " Dados = " << buffer.getTamanhoDosDados()+1 << endl;
			debugFile.close();
		}
#endif
	}
}