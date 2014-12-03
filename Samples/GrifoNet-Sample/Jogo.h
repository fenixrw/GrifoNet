#pragma once
#include "Personagem.h"

typedef struct{
	string* outData;
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

	ControleDeRede rede;	
	INSTANTIATE_DATA iData;
};