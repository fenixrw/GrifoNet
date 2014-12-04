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


Personagem::Personagem()
{
	createTime = std::chrono::system_clock::now();
	aliveTime = std::chrono::system_clock::now();
}

Personagem::~Personagem()
{
}


void Personagem::destruir()
{
	//função chamada antes de destruir um objeto
	//limpar variáveis e remover links com outros objetos caso existam
	cout << "Objeto Destruido" << endl;
}

void Personagem::atualizar()
{
	if(souDono())
	{
		//atualização do objeto original
		aliveTime = std::chrono::system_clock::now();
	}
	else
	{
		//atualização das réplicas do objeto
	}
}

void Personagem::serializarNovo(Buffer *buffer)
{
	//escrever no buffer as variáveis de criação do objeto
}

void Personagem::serializar(Buffer *buffer)
{
	//escrever no buffer as variáveis de atualização do objeto
}
void Personagem::deserializarNovo(Buffer *buffer)
{
	//ler do buffer as variáveis de criação do objeto
	cout << "Novo Objeto" << endl;
}
void Personagem::deserializar(Buffer *buffer)
{
	//ler do buffer as variáveis de atualização do objeto
}

unsigned short Personagem::getTipo()
{
	return Object_PLAYER;
}