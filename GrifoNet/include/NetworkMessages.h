/*
========================================================
GrifoNet: P2P Networking library based on SDL_net 2.0
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
#include <SDL_net.h>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

namespace grifo
{
	//enum {COMUM=0,STREAM,CONTROLE};
	enum {MessageType_COMMON=0,MessageType_OBJECT=1,MessageType_CONTROL=2,MessageType_CONNECTION=3};
	enum {TipoDeMensagem_COMUM=0,TipoDeMensagem_OBJETO=1,TipoDeMensagem_CONTROLE=2,TipoDeMensagem_CONEXAO=3};

	enum{AddresseeType_PEER=0, AddresseeType_SUPERPEER=1, AddresseeType_GROUP=2,AddresseeType_BROADCAST=3};
	enum{TipoDestinatario_PEER=0, TipoDestinatario_SUPERPEER=1, TipoDestinatario_GRUPO=2,TipoDestinatario_BROADCAST=3};

	//enum{ConnectionState_WAITING=0, ConnectionState_1stREQUEST=1, ConnectionState_REQUEST=2, ConnectionState_RESPONSE=3};

	//Connection Message Flags
	#define Flag_RESPONSE	1 << 0
	#define Flag_1stREQUEST 1 << 1
	#define Flag_SUPERPEER	1 << 2
	#define Flag_REDIRECT	1 << 3
	#define Flag_ACCEPT		1 << 4
	#define Flag_REFUSE		1 << 5


	#define FLAG_RESPOSTA		1 << 0
	#define FLAG_REQINICIAL		1 << 1 
	#define FLAG_SUPERPEER		1 << 2
	#define FLAG_REDIRECIONAR	1 << 3
	#define FLAG_ACEITAR		1 << 4
	#define FLAG_RECUSAR		1 << 5

	//Network Object Message Flags
	#define Flag_InstantiateNetworkObject 1 << 1 
	#define Flag_UpdateNetworkObject 1 << 2 
	#define Flag_DestroyNetworkObject 1 << 3 
	#define Flag_RequestNetworkObject 1 << 4 


	#define FLAG_INSTANCIAR 1 << 1 
	#define FLAG_ATUALIZAR 1 << 2 
	#define FLAG_DESTRUIR 1 << 3 

	//Control Messages
	//#define FLAG_RESPOSTA		1 << 0
	#define FLAG_GRUPO			1 << 1 
	//#define FLAG_ATUALIZAR	1 << 2 
	#define FLAG_BUS			1 << 3 

	static bool checkFlag(unsigned short flags, unsigned short flag)
	{
		if((flag & flags) == flag)
		{
			return true;
		}
		return false;
	}

	typedef struct _ProtocolInfo
	{
		unsigned short version:4;
		unsigned short type:2;
		unsigned short AddresseeType:2;
		unsigned short extension:2;
		unsigned short flags:6;//Flags are used to send information when requesting a connection or responding a connection request.
		/*** Connection Flags: **************************************************************************************************************************************************************
			-FLAG_RequestConnection: First Connection and Info Request to a SuperPeer 
			-FLAG_Response: Inform that the message is a Response
			-FLAG_Peer: This user is a Peer | Request connection to the 2nd Layer Network (Peers + 1 SuperPeer) as a Peer
			-FLAG_SuperPeer: This user is a Super Peer | Request connection to the SuperPeer's Network (1st Layer Network) as a SuperPeer
			-FLAG_SetSuperPeer: (Response only) Inform that the user requesting the connection is now a Super Peer [the data must contain the list of Super Peers in the SuperPeer's Network]
			-FLAG_SetServer: (Response only) Inform that the user requesting  the connection should act as a Server in the 2nd Layer Network
		*************************************************************************************************************************************************************************************/
	}ProtocolInfo;

	union uProtocolInfo
	{
		ProtocolInfo info;
		char stream[sizeof(ProtocolInfo)];
	};

	typedef struct _MessageHeader
	{
		ProtocolInfo info;
		unsigned short senderID:8;
		unsigned short addresseeID:8;

		/************************************************
		When requesting the first connection the user can 
		ask to be part of a specific group by setting the 
		adresseeType to group and setting the addresseeID 
		to the desired group ID.
		************************************************/
	}MessageHeader;

	union uMessageHeader
	{
		MessageHeader header;
		char stream[sizeof(MessageHeader)];
	};

	//GROUP Struct and Flags

	#define Flag_Group_RESPONSE 1 << 0
	#define Flag_Group_PASSWORD 1 << 1
	#define Flag_Group_PEERS	1 << 2
	#define Flag_Group_REFUSE	1 << 3

	#define Flag_Grupo_RESPOSTA 1 << 0
	#define Flag_Grupo_SENHA	1 << 1
	#define Flag_Grupo_PEERS	1 << 2
	#define Flag_Grupo_RECUSAR	1 << 3

	#define GroupMessageType_Create 0
	#define GroupMessageType_Add 1
	#define GroupMessageType_Remove 2

	typedef struct _GroupMessageHeader{
		unsigned short type:2;
		unsigned short flags:6;
		unsigned short groupId:8;
	}GroupMessageHeader;

	union uGroupMessageHeader
	{
		GroupMessageHeader header;
		char stream[sizeof(GroupMessageHeader)];
	};

	//typedef struct _MessageHeader_Ext1
	//{
	//	ProtocolInfo info;
	//	unsigned short senderID:16;
	//	unsigned short addresseeID:16;
	//}MessageHeader_Ext1;
	//
	//union uMessageHeader_Ext1
	//{
	//	MessageHeader_Ext1 header;
	//	char stream[sizeof(MessageHeader_Ext1)];
	//};
	//
	//
	//typedef struct _MessageHeader_Ext2
	//{
	//	ProtocolInfo info;
	//	unsigned int senderID:32;
	//	unsigned int addresseeID:32;
	//}MessageHeader_Ext2;
	//
	//union uMessageHeader_Ext2
	//{
	//	MessageHeader_Ext2 header;
	//	char stream[sizeof(MessageHeader_Ext2)];
	//};
	//
	//ERROR: sizeof == 24 when it should be == 18 
	//typedef struct _MessageHeader_Ext3
	//{
	//	ProtocolInfo info;
	//	unsigned long long senderID:64;
	//	unsigned long long addresseeID:64;
	//}MessageHeader_Ext3;
	//
	//union uMessageHeader_Ext3
	//{
	//	MessageHeader_Ext3 header;
	//	char stream[sizeof(MessageHeader_Ext3)];
	//};
	//
	//typedef struct _MessageConnectionRequest
	//{
	//	MessageHeader header;
	//	
	//}MessageConnectionRequest;
	//
	//union uMessageConnectionRequest
	//{
	//	MessageConnectionRequest header;
	//	char stream[sizeof(MessageConnectionRequest)];
	//};
}
