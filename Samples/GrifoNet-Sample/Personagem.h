#pragma once
#include <GrifoNet.h>
using namespace grifo;

enum AnimacaoPersonagem
{
	ANIMACAO_BAIXO,
	ANIMACAO_ESQ,
	ANIMACAO_DIR,
	ANIMACAO_CIMA
};

enum {Object_PLAYER=1};

class Personagem : public ObjetoDeRede
{
public:
	Personagem();
	~Personagem();
	
	void atualizar();

	unsigned short getTipo();
	void serializarNovo(Buffer *buffer);
	void deserializarNovo(Buffer *buffer);
	void serializar(Buffer *buffer);
	void deserializar(Buffer *buffer);
	
	void destruir();
	

private:

	
	
};

