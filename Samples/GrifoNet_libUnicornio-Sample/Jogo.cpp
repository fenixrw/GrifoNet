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
		SDL_Delay(300);
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
	//ObjetoDeRede *player = new Player();
	//if(data)
	//	/*((Jogo*)data)->*/textoInput.append("New Player ");

	/*
	
                {
                 "height":32,
                 "name":"Player",
                 "properties":
                    {

                    },
                 "type":"Player",
                 "visible":true,
                 "width":32,
                 "x":1120,
                 "y":224
                },  
	
	*/

	Personagem *player = new Personagem();

	if(data)
	{
		INSTANTIATE_DATA *iData = (INSTANTIATE_DATA*)data;
	
		ObjetoTile *tile = iData->mapa->criarObjeto();
		tile->setTipo("Player");
		tile->setNome("NetworkPlayer");
		tile->setVisivel(true);
		tile->setLargura(32);
		tile->setAltura(32);
		tile->setPos(0.5,0.5);
		player->setFPS(30);
		player->mapa = iData->mapa;
		player->texto = iData->texto;
		player->usarUDP = true;
		player->inicializar("player", tile);//iData->mapa->getObjeto("Player")
	}

	return (ObjetoDeRede*)(player);
}

Jogo::Jogo()
{
	iData.mapa = &mapa;
	iData.texto = &texto;
	pRede = &rede;
	rede._fCriarObjeto = intanciarObjetoDeRede;
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
	uniInicializar(800, 600, false);

	//	1)	carregar o tilemap
	mapa.carregar("dados/tilemaps/mapa_campo.json");

	//	2)	carregar spritesheet para o player
	recursos.carregarSpriteSheet("player", "dados/spritesheets/player.png", 4, 4);

	//	3)	carregar fonte para o texto
	recursos.carregarFonte("fonte colisao", "dados/fontes/F25_Bank_Printer.ttf", 16);

	//	4)	inicializar o personagem do player,
			//	passando o nome do spritesheet e o objeto que representa ele no mapa
	//player.inicializar("player", mapa.getObjeto("Player"));

	//	5)	inicializar texto
	texto.setFonte("fonte colisao");
	textoServer.setFonte("fonte colisao");
	textoMe.setFonte("fonte colisao");

	rede.inicializar(argc,argv);

	if(!rede.souRoot())
	{
		rede.adicionarObjeto(intanciarObjetoDeRede(Object_PLAYER,&iData));
		rede.conectar("localhost",2323);
	}

}

void Jogo::finalizar()
{
	//	O resto da finalização vem aqui (provavelmente, em ordem inversa a inicialização)!
	//	...
	rede.finalizar();

	//	12)	descarregar spritesheets e fontes
	recursos.descarregarTudo();

	//	13)	descarregar o tilemap
	mapa.descarregar();

	uniFinalizar();
}

void Jogo::executar()
{
	while(!teclado.soltou[TECLA_ESC] && !aplicacao.sair)
	{
		uniIniciarFrame();
		
		rede.atualizar();

		if(rede.souRoot())
			texto.setTexto("ROOT SERVER");
		else
			texto.setTexto(textoInput);
		
		////TESTAR E TRATAR COLISAO COM OBJETOS NO TILEMAP
		//testarColisoes();

		//desenhar o tilemap (player eh desenhado junto)
		mapa.desenhar();

		//desenhar texto da colisao
		texto.desenhar(res_x/2, res_y/5);

		if(!rede.souRoot())
		{
			textoMe.setTexto(toString(rede.getIdPeer()));
			textoMe.desenhar(res_x*0.05, res_y*0.05);
			textoServer.setTexto(textoInputServer);
			textoServer.desenhar(res_x/2, res_y/8);
		}

		uniTerminarFrame();
	}
}

void Jogo::testarColisoes()
{
	bool col_grama = colisaoGramaAlta();
	bool col_arvore = colisaoArvoreSeca();
	bool col_flor = colisaoFlorRara();

	////	se nao esta colidindo com nada, 
	//if(!col_grama && !col_arvore && !col_flor)
	//{
	//	//	limpa o texto
	//	texto.apagar();
	//}
}

bool Jogo::colisaoGramaAlta()
{
	//	se existe um objeto do tipo "Grama Alta" na posicao do player
	//if(mapa.existeObjetoDoTipoNaPos("Grama Alta", player->getX(), player->getY()))
	//{
	//	//	setar texto
	//	//texto.setTexto("Andando sobre grama alta!");

	//	//	esta colidindo
	//	return true;
	//}

	//	nao esta colidindo
	return false;
}

bool Jogo::colisaoArvoreSeca()
{
	//	se existe um objeto do tipo "Arvore Seca" na posicao do player
	//if(mapa.existeObjetoDoTipoNaPos("Arvore Seca", player->getX(), player->getY()))
	//{
	//	//	pega a propriedade chamada texto, contida do objeto do tipo "Arvore Seca"
	//	string txt = mapa.getObjetoDoTipoNaPos("Arvore Seca", player->getX(), player->getY())->getPropriedade("texto");

	//	//	setar texto
	//	//texto.setTexto(txt);

	//	//	esta colidindo
	//	return true;
	//}

	//	nao esta colidindo
	return false;
}

bool Jogo::colisaoFlorRara()
{
	//	se existe um objeto do tipo "Flor Rara" na posicao do player
	//if(mapa.existeObjetoDoTipoNaPos("Flor Rara", player->getX(), player->getY()))
	//{
	//	//	pega o nome da flor
	//	string nome_flor = mapa.getObjetoDoTipoNaPos("Flor Rara", player->getX(), player->getY())->getNome();

	//	//	pega a propriedade chamada texto, contida do objeto do tipo "Flor Rara"
	//	string txt_flor = mapa.getObjetoDoTipoNaPos("Flor Rara", player->getX(), player->getY())->getPropriedade("texto");

	//	//	junta as strings, colocando um ' - ' entre elas
	//	string txt = nome_flor;
	//	txt.append(" - ");
	//	txt.append(txt_flor);

	//	//	setar texto
	//	//texto.setTexto(txt);

	//	//	esta colidindo
	//	return true;
	//}

	//	nao esta colidindo
	return false;
}