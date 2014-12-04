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
		
	std::chrono::time_point<std::chrono::system_clock> createTime, aliveTime;
	
};

