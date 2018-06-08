// http://www.binarytides.com/udp-socket-programming-in-winsock/


#pragma once


#include "../essentials/main.h"

#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include <Windows.h>
#else
	#include <cstdio>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif
#include "../../dependencies/SDL2_net-2.0.1/include/SDL_net.h"


namespace NMaxestFramework { namespace NSystem
{
#ifdef MAXEST_FRAMEWORK_WINDOWS
    typedef SOCKET TSystemSocket;
#else
    typedef int TSystemSocket;
#endif

    //

	struct SIPAddress;
	class CTCPServer;
	class CTCPServerClientSocket;
	class CTCPClientSocket;
	class CUDPSocket;

	//

	void InitializeSockets();
	void DeinitializeSockets();
	void InitializeSockets2();
	void DeinitializeSockets2();

	struct SIPAddress
	{
		IPaddress ipAddress;

		SIPAddress() {}
		SIPAddress(const string& host, uint16 port) { Set(host, port); }

		void Set(const string& host, uint16 port) { MF_ASSERT(SDLNet_ResolveHost(&ipAddress, host.c_str(), port) != -1); }

		string Host() { return string(SDLNet_ResolveIP(&ipAddress)); }
		uint16 Port() { return SDLNet_Read16(&ipAddress.port); }

		bool operator == (const SIPAddress& other) { return (ipAddress.host == other.ipAddress.host && ipAddress.port == other.ipAddress.port); }
		bool operator != (const SIPAddress& other) { return !(*this == other); }
	};

	struct SIPAddress2
	{
		sockaddr_in address;

		SIPAddress2() {}
		SIPAddress2(const string& host, uint16 port) { Set(host, port); }

		void Set(const string& host, uint16 port)
		{
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = inet_addr(host.c_str());
			address.sin_port = htons(port);
		}

		string Host() { return string(inet_ntoa(address.sin_addr)); }
		uint16 Port() { return ntohs(address.sin_port); }

    #ifdef MAXEST_FRAMEWORK_WINDOWS
		bool operator == (const SIPAddress2& other) { return (address.sin_addr.S_un.S_addr == other.address.sin_addr.S_un.S_addr && address.sin_port == other.address.sin_port); }
    #else
		bool operator == (const SIPAddress2& other) { return (address.sin_addr.s_addr == other.address.sin_addr.s_addr && address.sin_port == other.address.sin_port); }
    #endif
		bool operator != (const SIPAddress2& other) { return !(*this == other); }
	};

	class CTCPServer
	{
	public:
		void Open(int maxSocketsCount, uint16 port);
		void Close();

		CTCPServerClientSocket* Poll(uint32 timeout); // returns pointer to new client (allocates memory); nullptr otherwise
		void Disconnect(const CTCPServerClientSocket& socket);

	public: // readonly
		SDLNet_SocketSet socketSet;
		TCPsocket socket;
	};

	class CTCPServerClientSocket
	{
	public:
		bool Recv(uint8* data, int& dataSize, int maxDataSize);
		int Send(const uint8* data, int dataSize);

	public: // readonly
		TCPsocket socket;
	};

	class CTCPClientSocket
	{
	public:
		void Open(SIPAddress& ipAddress);
		void Close();

		void Poll(uint32 timeout);

		bool Recv(uint8* data, int& dataSize, int maxDataSize);
		int Send(const uint8* data, int dataSize);

	public: // readonly
		SDLNet_SocketSet socketSet;
		TCPsocket socket;
	};

	class CUDPSocket
	{
	public:
		void Open(uint16 port, int recvPacketsCount, int sendPacketsCount, int maxPacketSize);
		void Close();

		void Poll(uint32 timeout);

		bool Recv(SIPAddress& ipAddress, uint8* data, int& dataSize, int recvPacketIndex = 0);
		void Send(const SIPAddress& ipAddress, uint8* data, int dataSize, int sendPacketIndex = 0);

	public: // readonly
		SDLNet_SocketSet socketSet;
		UDPsocket socket;
		vector<UDPpacket*> recvPackets;
		vector<UDPpacket*> sendPackets;
	};

	class CUDPSocket2
	{
	public:
		void Open(uint16 port);
		void Close();

		bool Recv(SIPAddress2& ipAddress, uint8* data, int& dataSize);
		void Send(const SIPAddress2& ipAddress, uint8* data, int dataSize);

	public: // readonly
		TSystemSocket sock;
		sockaddr_in address;
	};

	//

	inline void InitializeSockets()
	{
		MF_ASSERT(SDLNet_Init() != -1);
	}

	inline void DeinitializeSockets()
	{
		SDLNet_Quit();
	}

	inline void InitializeSockets2()
	{
    #ifdef MAXEST_FRAMEWORK_WINDOWS
		WSAData data;
		WSAStartup(MAKEWORD(2, 2), &data);
    #endif
	}

	inline void DeinitializeSockets2()
	{
    #ifdef MAXEST_FRAMEWORK_WINDOWS
		WSACleanup();
    #endif
	}

	//

	inline void CTCPServer::Open(int maxSocketsCount, uint16 port)
	{
		IPaddress ipAddress;

		MF_ASSERT(SDLNet_ResolveHost(&ipAddress, nullptr, port) != -1);
		socket = SDLNet_TCP_Open(&ipAddress);
		MF_ASSERT(socket != nullptr);

		socketSet = SDLNet_AllocSocketSet(maxSocketsCount);
		MF_ASSERT(socketSet != nullptr);
		MF_ASSERT(SDLNet_TCP_AddSocket(socketSet, socket) != -1);
	}

	inline void CTCPServer::Close()
	{
		MF_ASSERT(SDLNet_TCP_DelSocket(socketSet, socket) != -1);
		SDLNet_FreeSocketSet(socketSet);
	}

	inline CTCPServerClientSocket* CTCPServer::Poll(uint32 timeout)
	{
		MF_ASSERT(SDLNet_CheckSockets(socketSet, timeout) != -1);

		if (SDLNet_SocketReady(socket))
		{
			CTCPServerClientSocket* clientSocket = new CTCPServerClientSocket();

			clientSocket->socket = SDLNet_TCP_Accept(socket);
			MF_ASSERT(clientSocket->socket != nullptr);
			MF_ASSERT(SDLNet_TCP_AddSocket(socketSet, clientSocket->socket) != -1);

			return clientSocket;
		}
		else
		{
			return nullptr;
		}
	}

	inline void CTCPServer::Disconnect(const CTCPServerClientSocket& socket)
	{
		SDLNet_TCP_Close(socket.socket);

		MF_ASSERT(SDLNet_TCP_DelSocket(socketSet, socket.socket) != -1);
	}

	//

	inline bool CTCPServerClientSocket::Recv(uint8* data, int& dataSize, int maxDataSize)
	{
		if (SDLNet_SocketReady(socket))
		{
			dataSize = SDLNet_TCP_Recv(socket, data, maxDataSize);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline int CTCPServerClientSocket::Send(const uint8* data, int dataSize)
	{
		return SDLNet_TCP_Send(socket, data, dataSize);
	}

	//

	inline void CTCPClientSocket::Open(SIPAddress& ipAddress)
	{
		socket = SDLNet_TCP_Open(&ipAddress.ipAddress);
		MF_ASSERT(socket != nullptr);

		socketSet = SDLNet_AllocSocketSet(1);
		MF_ASSERT(socketSet != nullptr);
		MF_ASSERT(SDLNet_TCP_AddSocket(socketSet, socket) != -1);
	}

	inline void CTCPClientSocket::Close()
	{
		SDLNet_TCP_Close(socket);

		MF_ASSERT(SDLNet_TCP_DelSocket(socketSet, socket) != -1);
		SDLNet_FreeSocketSet(socketSet);
	}

	inline void CTCPClientSocket::Poll(uint32 timeout)
	{
		MF_ASSERT(SDLNet_CheckSockets(socketSet, timeout) != -1);
	}

	inline bool CTCPClientSocket::Recv(uint8* data, int& dataSize, int maxDataSize)
	{
		if (SDLNet_SocketReady(socket))
		{
			dataSize = SDLNet_TCP_Recv(socket, data, maxDataSize);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline int CTCPClientSocket::Send(const uint8* data, int dataSize)
	{
		return SDLNet_TCP_Send(socket, data, dataSize);
	}

	//

	inline void CUDPSocket::Open(uint16 port, int recvPacketsCount, int sendPacketsCount, int maxPacketSize)
	{
		socket = SDLNet_UDP_Open(port);
		MF_ASSERT(socket != nullptr);

		socketSet = SDLNet_AllocSocketSet(1);
		MF_ASSERT(socketSet != nullptr);
		MF_ASSERT(SDLNet_UDP_AddSocket(socketSet, socket) != -1);

		recvPackets.resize(recvPacketsCount);
		sendPackets.resize(sendPacketsCount);
		for (int i = 0; i < recvPacketsCount; i++)
		{
			recvPackets[i] = SDLNet_AllocPacket(maxPacketSize);
			MF_ASSERT(recvPackets[i] != nullptr);
		}
		for (int i = 0; i < sendPacketsCount; i++)
		{
			sendPackets[i] = SDLNet_AllocPacket(maxPacketSize);
			MF_ASSERT(sendPackets[i] != nullptr);
		}
	}

	inline void CUDPSocket::Close()
	{
		SDLNet_UDP_Close(socket);

		MF_ASSERT(SDLNet_UDP_DelSocket(socketSet, socket) != -1);
		SDLNet_FreeSocketSet(socketSet);

		for (uint i = 0; i < recvPackets.size(); i++)
		{
			recvPackets[i]->data = nullptr;
			SDLNet_FreePacket(recvPackets[i]);
		}
		for (uint i = 0; i < sendPackets.size(); i++)
		{
			sendPackets[i]->data = nullptr;
			SDLNet_FreePacket(sendPackets[i]);
		}
		recvPackets.clear();
		sendPackets.clear();
	}

	FORCE_INLINE void CUDPSocket::Poll(uint32 timeout)
	{
		MF_ASSERT(SDLNet_CheckSockets(socketSet, timeout) != -1);
	}

	FORCE_INLINE bool CUDPSocket::Recv(SIPAddress& ipAddress, uint8* data, int& dataSize, int recvPacketIndex)
	{
		if (SDLNet_SocketReady(socket))
		{
			int recv = SDLNet_UDP_Recv(socket, recvPackets[recvPacketIndex]);
			MF_ASSERT(recv >= 0);

			if (recv == 0)
				return false;

			ipAddress.ipAddress = recvPackets[recvPacketIndex]->address;
			memcpy(data, recvPackets[recvPacketIndex]->data, recvPackets[recvPacketIndex]->len);
			dataSize = recvPackets[recvPacketIndex]->len;

			return true;
		}
		else
		{
			return false;
		}
	}

	FORCE_INLINE void CUDPSocket::Send(const SIPAddress& ipAddress, uint8* data, int dataSize, int sendPacketIndex)
	{
		sendPackets[sendPacketIndex]->address = ipAddress.ipAddress;
		sendPackets[sendPacketIndex]->data = data;
		sendPackets[sendPacketIndex]->len = dataSize;

		int send = SDLNet_UDP_Send(socket, -1, sendPackets[sendPacketIndex]);
		MF_ASSERT(send != 0);
	}

	//

	FORCE_INLINE void CUDPSocket2::Open(uint16 port)
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		int b = ::bind(sock, (sockaddr*)&address, sizeof(address));
		MF_ASSERT(b != -1);

		int bufferSize = 4 * 1024 * 1024;
		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferSize, sizeof(bufferSize));
		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&bufferSize, sizeof(bufferSize));
	}

	FORCE_INLINE void CUDPSocket2::Close()
	{
    #ifdef MAXEST_FRAMEWORK_WINDOWS
		closesocket(sock);
    #else
        close(sock);
    #endif
	}

	FORCE_INLINE bool CUDPSocket2::Recv(SIPAddress2& ipAddress, uint8* data, int& dataSize)
	{
		// https://stackoverflow.com/questions/34242622/windows-udp-sockets-recvfrom-fails-with-error-10054

		MF_ASSERT(dataSize > 0);

    #ifdef MAXEST_FRAMEWORK_WINDOWS
		int fromLength = sizeof(ipAddress.address);
    #else
        socklen_t fromLength = sizeof(ipAddress.address);
    #endif

		dataSize = recvfrom(sock, (char*)data, dataSize, 0, (sockaddr*)&ipAddress.address, &fromLength);
		if (dataSize == -1)
		{
        #ifdef MAXEST_FRAMEWORK_WINDOWS
			if (WSAGetLastError() == WSAECONNRESET)
				return false;
        #endif
		}

		MF_ASSERT(dataSize >= 0);

		return dataSize > 0;
	}

	FORCE_INLINE void CUDPSocket2::Send(const SIPAddress2& ipAddress, uint8* data, int dataSize)
	{
		int s = sendto(sock, (char*)data, dataSize, 0, (sockaddr*)&ipAddress.address, sizeof(ipAddress.address));
		MF_ASSERT(s != -1);
	}
} }
