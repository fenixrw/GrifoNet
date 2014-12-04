#pragma once

#include "libUnicornio.h"
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

	void inicializar(string sheet, ObjetoTile* objeto);

	void atualizar();

	unsigned short getTipo();
	void serializarNovo(Buffer *buffer);
	void deserializarNovo(Buffer *buffer);
	void serializar(Buffer *buffer);
	void deserializar(Buffer *buffer);

	void moverPara(float x, float y);
	bool estaMovendo();

	void destruir();

	float getX();
	float getY();

	TileMap *mapa;
	Texto *texto;

private:
	ObjetoTile* objeto;
	Sprite spr;

	float proximo_x;
	float proximo_y;
	float x_anterior;
	float y_anterior;
	float interpolador;
	bool movendo;

	float vel;
};

