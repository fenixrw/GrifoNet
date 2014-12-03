#pragma once

#include "Buffer.h"
#include <chrono>

namespace grifo
{

	class ObjetoDeRede
	{
		friend class ControleDeRede;
	public:
		//Increment Object Id
		ObjetoDeRede(void);
		~ObjetoDeRede(void);
	
		virtual void atualizar() = 0;
		virtual unsigned short getTipo() = 0;
	
		virtual void serializar(Buffer *buffer) = 0;
		virtual void deserializar(Buffer *buffer) = 0;
	
		virtual void serializarNovo(Buffer *buffer) = 0;
		virtual void deserializarNovo(Buffer *buffer) = 0;
		virtual void destruir() = 0;

		unsigned short getID();

		bool souDono();
	
		bool usarUDP;
		bool enviar;

		void setFPS(unsigned int fps);

	private:

		void setId(unsigned short _id);
	
		std::chrono::time_point<std::chrono::system_clock> lastSendTime;

		//System variable DON'T CHANGE
		static bool NetworkInstantiate;
		static unsigned short NetworkInstantiateId;
		static unsigned short getNextId();

		bool checkSendTimer();

		bool dono;
		static unsigned short NextId;
		unsigned short id;

		double updateRate;
	};

}