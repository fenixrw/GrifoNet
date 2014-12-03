#include "Jogo.h"
#include <queue>

string textoInput="";
string textoInputServer="";
ControleDeRede *pRede=NULL;

Buffer mBuffer(512);

char charWelcome = 'w';

// Template function to convert most anything to a string
template<class T>
std::string toString(const T& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

#define DEBUGJOGADOR 
#define DEBUGGRUPO

vector<unsigned short> grupo;
bool grupoExiste = false;
bool welcomeMessageSentToGroup = false;

void atualizarGrupo(Grupo g)
{
	//textoInput = "Grupo ";
	//textoInput+= toString(g.id);
	//textoInput+= " atualizado";
#ifdef DEBUGGRUPO
	textoInputServer = "Grupo ";
	textoInputServer+= toString(g.id);
	textoInputServer+=" Atualizado [";
	for(set<unsigned short>::iterator i=g.membros.begin();i!=g.membros.end();i++)
	{
		if(i!=g.membros.begin())
		textoInputServer+=",";
		textoInputServer+= toString(*i);
	}
	textoInputServer += "]";
#endif
	if(pRede->getIdPeer() == 0 && !welcomeMessageSentToGroup)
	{
		welcomeMessageSentToGroup = true;
		mBuffer.init();
		mBuffer.escrever(charWelcome); 
		pRede->enviarMensagemGrupo(10,mBuffer);
		grupo.clear();
#ifdef DEBUGGRUPO
		textoInputServer += " <Enviada Mensagem>";
#endif
	}
}


void novoJogador(unsigned short id)
{
#ifdef DEBUGJOGADOR
	textoInput = "Jogador ";
	textoInput+= toString(id);
	textoInput+= " conectado";
#endif

	mBuffer.init();
	mBuffer.escrever(charWelcome);

	if(pRede->getIdPeer() == 0)
	{
		
		pRede->enviarMensagemPeer(id,mBuffer);

		if(!grupoExiste)
		if(id>1)
		{
			grupo.push_back(id);

			if(grupo.size()>2)
			{
				grupo.pop_back();
				pRede->grupoCriar(10,grupo);
				grupoExiste = true;
			}
		}
	}

}

void jogadorSaiu(unsigned short id)
{
#ifdef DEBUGJOGADOR
	textoInput = "Jogador ";
	textoInput+= toString(id);
	textoInput+= " desconectado";
#endif
}

void receberMensagem(MessageHeader header, Buffer &buffer)
{
	char c = '0';
	buffer.ler(c);

	if(c=='0')
	{
		textoInput = "Error";
	}
	else 
		
#if defined(DEBUGGRUPO) && !defined(DEBUGJOGADOR)
		if(header.info.AddresseeType==AddresseeType_GROUP)
#endif
	{		
		std::ostringstream stream;
		stream	<< "[" << ((header.info.AddresseeType==AddresseeType_GROUP)?"Grupo: ":"Peer: ") 
				<< ((header.info.AddresseeType==AddresseeType_GROUP)?header.addresseeID:header.senderID)
				<< " - " << ((c=='w')?"welcome":toString(c)) << "]";
		textoInputServer = stream.str();
	}
}

ObjetoDeRede* intanciarObjetoDeRede(unsigned int tipo, void *data)
{
	Personagem *player = new Personagem();

	if(data)
	{
		INSTANTIATE_DATA *iData = (INSTANTIATE_DATA*)data;
	
		player->setFPS(30);
		player->usarUDP = true;
	}

	return (ObjetoDeRede*)(player);
}

Jogo::Jogo()
{
	iData.outData = &textoInput;
	pRede = &rede;
	rede._fCriarObjeto = &intanciarObjetoDeRede;
	rede._dadosCriarObjeto = &iData;
	rede._fSignalNovoJogador = &novoJogador;
	rede._fSignalJogadorDesconectou = &jogadorSaiu;
	rede._fTratarMensagensRecebidas = &receberMensagem;
	rede._fAtualizarGrupo = &atualizarGrupo;
}

Jogo::~Jogo()
{
}

void Jogo::inicializar(int argc, char** argv)
{
	string rootIP = "localhost";
	Uint16 rootPort = 2323;

	rede.inicializar(argc,argv);

	if(!rede.souRoot())
	{
		rede.adicionarObjeto(intanciarObjetoDeRede(Object_PLAYER,&iData));
		rede.conectar(rootIP,rootPort);
	}

}

void Jogo::finalizar()
{
	//	O resto da finalização vem aqui (provavelmente, em ordem inversa a inicialização)!
	//	...
	rede.finalizar();
}

void Jogo::executar()
{
	while(true)
	{
		rede.atualizar();
	}
}