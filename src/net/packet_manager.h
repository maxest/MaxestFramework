#pragma once
#ifdef MAXEST_FRAMEWORK_DESKTOP


#include "../essentials/main.h"
#include "../system/main.h"
#include "../math/main.h"


namespace NMaxestFramework { namespace NNet
{
	class CPacketManager
	{
	private:
		static const int cResendTimeout = 10000; // 10 ms
		static const int cReliablePacketToSendDeletionTimeout = 10000000; // 10 secs
		static const int cReceivedReliablePacketDeletionTimeout = 2 * cReliablePacketToSendDeletionTimeout; // 20 secs

	private:
		struct SPacketData
		{
			uint8* data;
			int dataSize; // with header

			SPacketData()
			{
				data = nullptr;
				dataSize = 0;
			}

			void Destroy()
			{
				SAFE_DELETE_ARRAY(this->data);
			}

			void SetUnreliable(uint8* data, int dataSize)
			{
				SAFE_DELETE_ARRAY(this->data);

				this->dataSize = dataSize + 1;

				this->data = new uint8[this->dataSize];
				this->data[0] = 0;
				memcpy(this->data + 1, data, dataSize);
			}
		
			void SetReliable(uint8* data, int dataSize)
			{
				SAFE_DELETE_ARRAY(this->data);

				this->dataSize = dataSize + 5;

				int32 guid = NMath::Random32(1, cIntMax);

				this->data = new uint8[this->dataSize];
				this->data[0] = 1;
				memcpy(this->data + 1, &guid, 4);
				memcpy(this->data + 5, data, dataSize);
			}

			void SetACK(int32 guid)
			{
				SAFE_DELETE_ARRAY(this->data);

				this->dataSize = 5;

				this->data = new uint8[5];
				this->data[0] = 2;
				memcpy(this->data + 1, &guid, 4);
			}

			bool IsUnreliable()
			{
				return data[0] == 0;
			}

			bool IsReliable()
			{
				return data[0] == 1;
			}

			bool IsACK()
			{
				return data[0] == 2;
			}

			void Data(uint8* data, int& dataSize)
			{
				if (IsUnreliable())
				{
					dataSize = this->dataSize - 1;
					memcpy(data, this->data + 1, dataSize);
				}
				else if (IsReliable())
				{
					dataSize = this->dataSize - 5;
					memcpy(data, this->data + 5, dataSize);
				}
				else
				{
					dataSize = 0;
				}
			}

			int32 GUID()
			{
				if (IsReliable() || IsACK())
				{
					int32 value;
					memcpy(&value, data + 1, 4);
					return value;
				}
				else
				{
					return 0;
				}
			}
		};

		struct SReliablePacketToSend
		{
			NSystem::CUDPSocket* udpSocket;
			NSystem::SIPAddress ipAddress;
			SPacketData packetData;
			uint64 creationTimestamp;
			uint64 resendTimestamp;
		};

		struct SReceivedReliablePacket
		{
			int32 guid;
			uint64 timestamp;
		};

	public:
		void Create(int maxPacketSize);
		void Destroy();

		void Send(bool isReliable, NSystem::CUDPSocket* udpSocket, NSystem::SIPAddress ipAddress, uint8* data, int dataSize);
		bool Recv(NSystem::CUDPSocket* udpSocket, NSystem::SIPAddress& ipAddress, uint8* data, int& dataSize);

		void Update();

	private:
		uint8* tempData;
		int tempDataSize;

		vector<SReliablePacketToSend> reliablePacketsToSend;
		vector<SReceivedReliablePacket> receivedReliablePackets;
	};
} }


inline void NMaxestFramework::NNet::CPacketManager::Create(int maxPacketSize)
{
	tempData = new uint8[maxPacketSize];
}


inline void NMaxestFramework::NNet::CPacketManager::Destroy()
{
	SAFE_DELETE_ARRAY(tempData);
}


inline void NMaxestFramework::NNet::CPacketManager::Send(bool isReliable, NSystem::CUDPSocket* udpSocket, NSystem::SIPAddress ipAddress, uint8* data, int dataSize)
{
	if (isReliable)
	{
		SReliablePacketToSend reliablePacket;
		reliablePacket.udpSocket = udpSocket;
		reliablePacket.ipAddress = ipAddress;
		reliablePacket.packetData.SetReliable(data, dataSize);
		reliablePacket.creationTimestamp = NSystem::TickCount();
		reliablePacket.resendTimestamp = reliablePacket.creationTimestamp;

		reliablePacketsToSend.push_back(reliablePacket);
	}
	else
	{
		SPacketData packetData;
		packetData.SetUnreliable(data, dataSize);
		udpSocket->Send(ipAddress, packetData.data, packetData.dataSize);
		packetData.Destroy();
	}
}


inline bool NMaxestFramework::NNet::CPacketManager::Recv(NSystem::CUDPSocket* udpSocket, NSystem::SIPAddress& ipAddress, uint8* data, int& dataSize)
{
	if (udpSocket->Recv(ipAddress, tempData, tempDataSize))
	{
		SPacketData packetData;
		packetData.data = tempData;
		packetData.dataSize = tempDataSize;

		if (packetData.IsUnreliable())
		{
			packetData.Data(data, dataSize);
			return true;
		}
		else if (packetData.IsReliable())
		{
			bool found = false;

			for (uint i = 0; i < receivedReliablePackets.size(); i++)
			{
				if (receivedReliablePackets[i].guid == packetData.GUID())
				{
					found = true;
					break;
				}
			}

			// send ACK
			SPacketData ackPacketData;
			ackPacketData.SetACK(packetData.GUID());
			udpSocket->Send(ipAddress, ackPacketData.data, ackPacketData.dataSize);

			if (!found)
			{
				SReceivedReliablePacket reliablePacket;
				reliablePacket.guid = packetData.GUID();
				reliablePacket.timestamp = NSystem::TickCount();

				receivedReliablePackets.push_back(reliablePacket);

				packetData.Data(data, dataSize);
			}
			
			return !found;
		}
		else if (packetData.IsACK())
		{
			for (uint i = 0; i < reliablePacketsToSend.size(); i++)
			{
				if (reliablePacketsToSend[i].packetData.GUID() == packetData.GUID())
				{
					reliablePacketsToSend.erase(reliablePacketsToSend.begin() + i);
					break;
				}
			}

			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


inline void NMaxestFramework::NNet::CPacketManager::Update()
{
	uint64 tickCount = NSystem::TickCount();

	// send all reliable packets for which we have not received ACKs yet
	for (uint i = 0; i < reliablePacketsToSend.size(); i++)
	{
		if (tickCount - reliablePacketsToSend[i].resendTimestamp > cResendTimeout)
		{
			reliablePacketsToSend[i].udpSocket->Send(reliablePacketsToSend[i].ipAddress, reliablePacketsToSend[i].packetData.data, reliablePacketsToSend[i].packetData.dataSize);
			reliablePacketsToSend[i].resendTimestamp = tickCount;
		}
	}

	// delete one timeoutted reliable packet that was to be sent but has not received an ACK
	for (uint i = 0; i < reliablePacketsToSend.size(); i++)
	{
		if (tickCount - reliablePacketsToSend[i].creationTimestamp > cReliablePacketToSendDeletionTimeout)
		{
			reliablePacketsToSend.erase(reliablePacketsToSend.begin() + i);
			break;
		}
	}

	// delete one reliable packet that we received (we stop keeping track of it, hoping that its ACK has been delivered)
	for (uint i = 0; i < receivedReliablePackets.size(); i++)
	{
		if (tickCount - receivedReliablePackets[i].timestamp > cReceivedReliablePacketDeletionTimeout)
		{
			receivedReliablePackets.erase(receivedReliablePackets.begin() + i);
			break;
		}
	}
}


#endif
