#pragma once
#include "Personagem.h"

typedef struct{
	TileMap *mapa;
	Texto *texto;
}INSTANTIATE_DATA;

class Jogo
{
public:
	Jogo();
	~Jogo();

	void inicializar(int argc, char** argv);
	void finalizar();

	void executar();


private:
	//	metodos para colisao
	void testarColisoes();
	bool colisaoGramaAlta();
	bool colisaoArvoreSeca();
	bool colisaoFlorRara();

	TileMap mapa;
	//Personagem *player;

	ControleDeRede rede;
	
	Texto texto;
	Texto textoServer;
	Texto textoMe;
	Texto textoPortas;

	INSTANTIATE_DATA iData;
};