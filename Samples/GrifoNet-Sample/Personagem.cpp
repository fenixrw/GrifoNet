#include "Personagem.h"


Personagem::Personagem()
{
}

Personagem::~Personagem()
{
}


void Personagem::destruir()
{
}

void Personagem::atualizar()
{
	if(souDono())
	{
		
	}
	else
	{

	}
}


void Personagem::serializarNovo(Buffer *buffer)
{
}
void Personagem::serializar(Buffer *buffer)
{
	
}
void Personagem::deserializarNovo(Buffer *buffer)
{

}
void Personagem::deserializar(Buffer *buffer)
{
	
}
unsigned short Personagem::getTipo()
{
	return Object_PLAYER;
}