#ifndef __NETWORK_TYPES_H
#define __NETWORK_TYPES_H

#include "RakNetDefines.h"
#include "Export.h"

namespace RakNet { class BitStream; };

#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

typedef unsigned char UniqueIDType;
typedef unsigned short PlayerIndex;
typedef unsigned char RPCIndex;
const int MAX_RPC_MAP_SIZE=((RPCIndex)-1)-1;
const int UNDEFINED_RPC_INDEX=((RPCIndex)-1);
typedef unsigned char MessageID;

typedef unsigned int RakNetTime;
typedef long long RakNetTimeNS;

#pragma pack(push, 1) // Упаковываем только сетевые ID
struct RAK_DLL_EXPORT PlayerID
{
	unsigned int binaryAddress;
	unsigned short port;

	char *ToString(bool writePort=true) const;
	void SetBinaryAddress(const char *str);
	PlayerID& operator = ( const PlayerID& input );
	bool operator==( const PlayerID& right ) const;
	bool operator!=( const PlayerID& right ) const;
	bool operator > ( const PlayerID& right ) const;
	bool operator < ( const PlayerID& right ) const;
};
#pragma pack(pop)

// Packet НЕ упаковываем (64-бит выравнивание обязательно!)
struct Packet
{
	PlayerIndex playerIndex;
	PlayerID playerId;
	unsigned int length;
	unsigned int bitSize;
	unsigned char* data;
	bool deleteData;
};

#pragma pack(push, 1)
struct RAK_DLL_EXPORT NetworkID
{
	PlayerID playerId;
	unsigned short localSystemId;
	static bool peerToPeerMode;

	static bool IsPeerToPeerMode(void);
	static void SetPeerToPeerMode(bool isPeerToPeer);
	NetworkID& operator = ( const NetworkID& input );
	bool operator==( const NetworkID& right ) const;
	bool operator!=( const NetworkID& right ) const;
	bool operator > ( const NetworkID& right ) const;
	bool operator < ( const NetworkID& right ) const;
};

class RakPeerInterface;
struct RPCParameters
{
	unsigned char *input;
	unsigned int numberOfBitsOfData;
	PlayerID sender;
	RakPeerInterface *recipient;
	RakNet::BitStream *replyToSender;
};
#pragma pack(pop)

const PlayerID UNASSIGNED_PLAYER_ID = {0xFFFFFFFF, 0xFFFF};
const PlayerIndex UNASSIGNED_PLAYER_INDEX = 65535;
const int PING_TIMES_ARRAY_SIZE = 5;
#define PlayerID_Size 6

#endif
