#include "Jogo.h"

int main(int argc, char** argv)
{
	Jogo jogo;

	jogo.inicializar(argc, argv);
	jogo.executar();
	jogo.finalizar();
	return 0;
}