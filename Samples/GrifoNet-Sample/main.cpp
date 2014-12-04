/*
========================================================
GrifoNet-Sample
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

#include "Personagem.h"

//Controle de Rede
ControleDeRede rede;

//Lista de Jogadores
list<unsigned short> connectedPeers;

//Lista de Usuários para criar um Grupo
vector<unsigned short> grupo;
bool grupoExiste = false;
bool welcomeMessageSentToGroup = false;

Buffer mBuffer(512);
char charWelcome = 'w';

bool run = true;

typedef struct{
	string *strData;
	int *intData;
}INSTANTIATE_DATA;

template<class T>
std::string toString(const T& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

//Função chamada quando um grupo é Criado, Atualizado ou Removido
void atualizarGrupo(Grupo g)
{
	cout << "Grupo " << g.id << " Atualizado [";
	for(set<unsigned short>::iterator i=g.membros.begin();i!=g.membros.end();i++)
	{
		if(i!=g.membros.begin())
			cout << ",";
		cout << *i;
	}
	cout << "]" << endl;

	if(rede.getIdPeer() == 0 && !welcomeMessageSentToGroup)
	{
		welcomeMessageSentToGroup = true;
		mBuffer.init();
		mBuffer.escrever(charWelcome); 
		rede.enviarMensagemGrupo(10,mBuffer);
		grupo.clear();
		cout << "<Mensagem Enviada para o Grupo>" << endl;
	}
}

//Função chamada quando um novo usuário se conecta à sala
void novoJogador(unsigned short id)
{
	cout << "Jogador " << id << " conectado" << endl;

	mBuffer.init();
	mBuffer.escrever(charWelcome);

	if(rede.getIdPeer() == 0)
	{
		
		rede.enviarMensagemPeer(id,mBuffer);

		if(!grupoExiste)
		if(id>1)
		{
			grupo.push_back(id);

			if(grupo.size()>2)
			{
				grupo.pop_back();
				rede.grupoCriar(10,grupo);
				grupoExiste = true;
			}
		}
	}

}

//Função chamada quando um usuário é desconectado
void jogadorSaiu(unsigned short id)
{
	cout << "Jogador " << id << " desconectado" << endl;
}

//Função chamada quando uma Mensagem Comum é recebida
void receberMensagem(MessageHeader header, Buffer &buffer)
{
	char c = '0';
	buffer.ler(c);

	if(c=='0')
	{
		run = false;
	}
	else 
	{		
		cout	<< "[" << ((header.info.AddresseeType==AddresseeType_GROUP)?"Grupo: ":"Peer: ") 
				<< ((header.info.AddresseeType==AddresseeType_GROUP)?header.addresseeID:header.senderID)
				<< " - " << ((c=='w')?"welcome":toString(c)) << "]" << endl;
	}
}

//Função chamada quando uma réplica de ObjetoDeRede deve ser criada
ObjetoDeRede* intanciarObjetoDeRede(unsigned int tipo, void *data)
{
	Personagem *player = new Personagem();

	if(data)
	{
		INSTANTIATE_DATA *iData = (INSTANTIATE_DATA*)data;
	
		player->setFPS(1);
		player->usarUDP = true;
	}

	return (ObjetoDeRede*)(player);
}

int main(int argc, char** argv)
{
	INSTANTIATE_DATA iData;	
	string dados = "Hello World!";
	int num = 23;
	iData.strData = &dados;
	iData.intData = &num;

	//Cadastrar funções no Controle de Rede
	rede._fCriarObjeto = &intanciarObjetoDeRede;
	rede._dadosCriarObjeto = &iData;
	rede._fSignalNovoJogador = &novoJogador;
	rede._fSignalJogadorDesconectou = &jogadorSaiu;
	rede._fTratarMensagensRecebidas = &receberMensagem;
	rede._fAtualizarGrupo = &atualizarGrupo;

	//Inicializar a Rede
	rede.inicializar(argc, argv);

	if(rede.souRoot())
	{
		//Se iniciou como Servidor Raiz apenas aguarda conexões
		cout << "Servidor Raiz Iniciado" << endl;
	}
	else
	{
		//Se iniciou como um usuário normal tenta conectar-se ao Servidor Raiz
		cout << "Conectando ao Servidor Raiz" << endl;
		rede.conectar("localhost",2000);

		//Adiciona um Objeto de Rede ao Controle de Rede para que este seja replicado e atualizado
		rede.adicionarObjeto(intanciarObjetoDeRede(Object_PLAYER,&iData));
	}

	while(run)
	{
		//Atualiza a rede, suas conexões e objetos
		rede.atualizar();
	}

	//finaliza a rede
	rede.finalizar();
	return 0;
}