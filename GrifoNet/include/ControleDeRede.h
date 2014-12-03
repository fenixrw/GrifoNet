#pragma once

#include "ObjetoDeRede.h"
#include "NetworkMessages.h"
#include "Buffer.h"
#include "Peer.h"
#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
namespace grifo
{
	class ControleDeRede
	{
	public:
		ControleDeRede(void);
		~ControleDeRede(void);
		void inicializar(int argc, char** argv);//http://tclap.sourceforge.net/manual.html
		void finalizar();
		void adicionarObjeto(ObjetoDeRede* obj);
		void conectar(string host, Uint16 port, bool comoSuperPeer = false);
		void conectar(IPaddress ip, bool comoSuperPeer = false);
		void desconectar();
		void atualizar();
		bool enviarMensagemPeer(unsigned short peerID, Buffer &message, bool udp = false);
		bool enviarMensagemGrupo(unsigned short grupoID, Buffer &message, bool udp = false);
		void grupoCriar(unsigned short grupoID, vector<unsigned short> membrosPeerID);
		void grupoEntrar(unsigned short grupoID);
		void grupoAdicionarMembros(unsigned short grupoID, vector<unsigned short> membrosPeerID);
		void grupoSair(unsigned short grupoID);
		void grupoExcluirMembros(unsigned short grupoID, vector<unsigned short> membrosPeerID);
		bool souRoot();
		bool souSuperPeer();
		unsigned short getIdPeer();
	
		void (*_fSignalNovoJogador)(unsigned short);
		void (*_fSignalJogadorDesconectou)(unsigned short);

		void (*_fTratarMensagensRecebidas)(MessageHeader,Buffer&);
	
		void (*_fAtualizarGrupo)(Grupo);

		ObjetoDeRede* (*_fCriarObjeto)(unsigned int /*Object Type*/, void*/*Game Data - For use on the Instantiate method (Suggestion: struct with pointers to World, Tilemap, SpriteSet, ...)*/);
		void* _dadosCriarObjeto;

	private:

		void adminRemovePeerFromAllGroups(unsigned short pID);

		bool _primeiraConexao;

		void receberConexoes();
		void tratarMensagens();
		void atualizarObjetos();
		void instanciarTodosObjetosParaPeer(Peer* peer);
		void enviarMensagemInstanciarObjeto(ObjetoDeRede* obj, Peer* peer=NULL);

		void tratarMensagemControleGrupo(MessageHeader header, Buffer &message);

		Peer* encontrarSuperPeerDisponivel();

		Peer* _rootPeer;
		Peer* _adminPeer;

		SDLNet_SocketSet _socketSet;
		list<Peer> _conexoesRecebidas;
		list<Peer> _conexoesSolicitadas;
	
		bool _souRoot;
		TCPsocket _listener;
		IPaddress _listenerIp;
		Uint16 _tcpPorta;
		Uint16 _tcpPortaN;
		UDPsocket _udpOut;
		Uint16 _udpOutPorta;
		Uint16 _udpOutPortaN;
		UDPsocket _udpIn;
		Uint16 _udpInPorta;
		Uint16 _udpInPortaN;
		unsigned int _maxSuperPeer;
		unsigned int _maxPeer;
		unsigned short _ultimoSuperPeerID;

		bool _souSuperPeer;
		bool _updateSuperPeerInfo;
		unsigned short _idSuperPeer:8;
		map<unsigned short, Peer> _superPeers;
		unsigned short _chaveRedeSuperPeer;
		unsigned short _ultimoPeerID;

		unsigned short _idPeer:8;
		map<unsigned short, Peer> _peers;
		unsigned short _chaveSalaDeJogo;
		map<unsigned short, ObjetoDeRede*> _objetos;
		map<unsigned short, Grupo> _grupos;

		Buffer buffer;
	};
}