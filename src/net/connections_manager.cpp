#include "connections_manager.h"


using namespace NMaxestFramework;


const uint64 cTimeout = 5000000;


THREAD_FUNCTION_RETURN_VALUE NMaxestFramework::NNet::CConnectionsManager::RecvThread(void* threadData)
{
	CConnectionsManager* connectionsManager = (CConnectionsManager*)threadData;

	NSystem::SIPAddress2 ip;
	uint8 data[cUDPPacketSize];

	for (;;)
	{
		int dataSize = cUDPPacketSize;

		if (connectionsManager->mySocket.Recv(ip, data, dataSize))
		{
			uint64 tickCount = NSystem::TickCount();

			// find connection's index by IP
			int connectionIndex = -1;
			bool connectionIsNew = true;
			for (uint i = 0; i < connectionsManager->connectionIds.size(); i++)
			{
				if (connectionsManager->connectionIds[i]->ip == ip)
				{
					connectionsManager->connectionIds[i]->lastRecvPacketTimestamp = tickCount;
					connectionIndex = i;
					connectionIsNew = false;
					break;
				}
			}
			if (connectionIndex == -1)
			{
				TConnectionId connection = new SConnection();
				connection->ip = ip;
				connection->lastRecvPacketTimestamp = tickCount;
				connectionsManager->connectionIds.push_back(connection);

				connectionIndex = (int)connectionsManager->connectionIds.size() - 1;
			}

			MF_ASSERT(connectionIndex >= 0 && connectionIndex < (int)connectionsManager->connectionIds.size());

			SPacket packet;
			packet.connectionId = connectionsManager->connectionIds[connectionIndex];
			packet.connectionIsNew = connectionIsNew;
			memcpy(packet.data, data, dataSize);
			packet.dataSize = dataSize;

			NSystem::MutexLock(connectionsManager->recvMutex);
			connectionsManager->recvPackets.push_back(packet);
			NSystem::MutexUnlock(connectionsManager->recvMutex);
		}
	}

	return 0;
}


void NNet::CConnectionsManager::Create(uint16 port)
{
	mySocket.Open(port);
	recvThreadHandle = NSystem::ThreadCreate(RecvThread, this);

	recvMutex = NSystem::MutexCreate();
}


void NNet::CConnectionsManager::Destroy()
{
	mySocket.Close();
	NSystem::ThreadDestroy(recvThreadHandle);

	NSystem::MutexDestroy(recvMutex);
	for (uint i = 0; i < connectionIds.size(); i++)
		delete connectionIds[i];
	connectionIds.clear();
	recvPackets.clear();
}


void NNet::CConnectionsManager::DestroyClient(TConnectionId connectionId)
{
	for (uint i = 0; i < connectionIds.size(); i++)
	{
		if (connectionIds[i] == connectionId)
		{
			delete connectionIds[i];
			connectionIds.erase(connectionIds.begin() + i);
		}
	}
}


bool NNet::CConnectionsManager::IsTimeoutted(uint64 tickCount, TConnectionId connectionId)
{
	return (tickCount - connectionId->lastRecvPacketTimestamp > cTimeout);
}


void NNet::CConnectionsManager::Send(TConnectionId connectionId, uint8* data, int dataSize)
{
	mySocket.Send(connectionId->ip, data, dataSize);
}


bool NNet::CConnectionsManager::PopRecvPacket(SPacket* packet)
{
	MF_ASSERT(packet != nullptr);

	if (recvPackets.size() == 0)
		return false;

	NSystem::MutexLock(recvMutex);
	*packet = recvPackets.front();
	recvPackets.pop_front();
	NSystem::MutexUnlock(recvMutex);

	return true;
}
