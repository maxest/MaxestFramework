#pragma once
#ifdef MAXEST_FRAMEWORK_DESKTOP


#include "../essentials/main.h"
#include "../system/main.h"
#include "../math/main.h"


namespace NMaxestFramework { namespace NNet
{
	const int cUDPPacketSize = 1450;

	struct SConnection
	{
		NSystem::SIPAddress2 ip;
		uint64 lastRecvPacketTimestamp;
	};

	typedef SConnection* TConnectionId;

	struct SPacket
	{
		TConnectionId connectionId;
		bool connectionIsNew;
		uint8 data[cUDPPacketSize];
		int dataSize;
	};

	class CConnectionsManager
	{
	private:
		static THREAD_FUNCTION_RETURN_VALUE RecvThread(void* threadData);

	public:
		void Create(uint16 port);
		void Destroy();

		void Close(TConnectionId connectionId);

		void Send(TConnectionId connectionId, uint8* data, int dataSize);
		bool PopRecvPacket(SPacket* packet);

	private:
		NSystem::CUDPSocket2 mySocket;
		NSystem::TThreadHandle recvThreadHandle;

		NSystem::TMutexHandle recvMutex;
		vector<TConnectionId> connectionIds;
		deque<SPacket> recvPackets;
	};
} }


#endif
