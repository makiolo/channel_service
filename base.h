#ifndef NETWORKBASE_H
#define NETWORKBASE_H

#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <raknet/MessageIdentifiers.h>
#include <raknet/RakPeerInterface.h>
#include <raknet/RakNetTypes.h>
#include <raknet/BitStream.h>
#include <fast-event-system/bind.h>
#include <fast-event-system/clock.h>
#include <fast-event-system/async_fast.h>
#include <cppunix/coroutine.h>
#include "serialize.h"
#include "deserialize.h"
#include "api.h"

#define DEFAULT_CONNECTION_TIMEOUT 5000
#define MAX_CLIENTS_DEFAULT 16000
#define ROOM_DEFAULT "default"
#define DEFAULT_RELIABLE_TIMEOUT 1000

namespace ser {

using string = RakNet::RakString;
using stream = RakNet::BitStream;
// version, pipe, sender
using net_channel = fes::bind<int, ser::stream, ser::string>;
using net_connection = fes::connection<int, ser::stream, ser::string>;

enum InternalMessages
{
	// received by server
	NET_MESSAGE_BROADCAST = ID_USER_PACKET_ENUM + 1,
	NET_MESSAGE_FORWARD = ID_USER_PACKET_ENUM + 2,
	// received by both (client and server)
	NET_MESSAGE_DATA = ID_USER_PACKET_ENUM + 3,
	NET_MESSAGE_DISCOVERY = ID_USER_PACKET_ENUM + 4,
	NET_MESSAGE_NEW_CLIENT = ID_USER_PACKET_ENUM + 5,
	NET_MESSAGE_DISCONNECT_CLIENT = ID_USER_PACKET_ENUM + 6,
	// entrar y salir de un canal
	NET_MESSAGE_ROOM_JOIN = ID_USER_PACKET_ENUM + 7,
	NET_MESSAGE_ROOM_LEAVE = ID_USER_PACKET_ENUM + 8,
	// hablar a través de un canal
	NET_MESSAGE_ROOM_FORWARD = ID_USER_PACKET_ENUM + 9,
	// enviar desde el servidor al cliente
	NET_MESSAGE_ROOM_DATA = ID_USER_PACKET_ENUM + 10,
	//
	NET_MESSAGE_CUSTOM = ID_USER_PACKET_ENUM + 11
};

class serializer_API network_base
{
public:
	explicit network_base(const ser::string& alias);
    virtual ~network_base();
    network_base(const network_base&) = delete;
    network_base& operator=(const network_base&) = delete;

	void update(fes::deltatime tmax = fes::deltatime(16))
	{
		fes::marktime timeout = fes::high_resolution_clock() + tmax;
		bool has_next = true;
		while(has_next && (fes::high_resolution_clock() <= timeout))
		{
			has_next = dispatch_one();
		}
	}

	void fortime(fes::deltatime time)
	{
		auto mark = fes::high_resolution_clock() + time;
		while(fes::high_resolution_clock() <= mark)
		{
			dispatch_one();
		}
	}

	void update(cu::push_type<void>& yield, fes::deltatime tmax = fes::deltatime(16))
	{
		fes::marktime timeout = fes::high_resolution_clock() + tmax;
		bool has_next = true;
		while(has_next && (fes::high_resolution_clock() <= timeout))
		{
			has_next = dispatch_one();
			yield();
		}
	}

	void fortime(cu::push_type<void>& yield, fes::deltatime time)
	{
		auto mark = fes::high_resolution_clock() + time;
		while(fes::high_resolution_clock() <= mark)
		{
			dispatch_one();
			yield();
		}
	}

	bool dispatch_one();

	virtual void on_new_incoming_connection(RakNet::Packet* packet) { ; }
	virtual void on_package(RakNet::Packet* packet) = 0;

	template <typename ... Args>
	void unpack(RakNet::Packet* packet, Args& ... data) const
	{
		RakNet::BitStream pipe(packet->data, packet->length, false);
		read_header(pipe);
		ser::deserialize(pipe, data...);
	}

	void read_header(ser::stream& stream) const
	{
		stream.IgnoreBytes(sizeof(RakNet::MessageID));
	}

	void write_header(ser::stream& stream, RakNet::MessageID id_interface) const
	{
		// static_assert( (sizeof(RakNet::MessageID) == sizeof(id_interface)) );
		stream.Write(id_interface);
	}

	ser::string get_guid_sender(RakNet::Packet* packet) const
	{
		auto& guid =  _peer->GetGuidFromSystemAddress(packet->systemAddress);
		return ser::string( guid.ToString() );
	}

	ser::string get_guid() const
	{
		auto& guid = _peer->GetMyGUID();
		return ser::string( guid.ToString() );
	}

	ser::string get_name() const
	{
		auto& guid = _peer->GetMyGUID();
		auto client = _peer->GetSystemAddressFromGuid(guid);
		return ser::string( client.ToString() );
	}

	RakNet::SystemAddress convert_to_address(const ser::string& guid) const
	{
		RakNet::RakNetGUID rak_guid;
		rak_guid.FromString(guid.C_String());
		return _peer->GetSystemAddressFromGuid(rak_guid);
	}

	ser::string convert_to_guid(const RakNet::SystemAddress& addr) const
	{
		RakNet::RakNetGUID guid = _peer->GetGuidFromSystemAddress(addr);
		return ser::string( guid.ToString() );
	}

	auto& get_channel(int port) { return _requests[port]; }
	auto& get_room(const std::string& room) { return _rooms[room]; }
	const auto& get_alias() const {return _alias;}
	bool valid_alias() const { return _alias.GetLength() > 0; }

	ser::string get_alias(const ser::string& guid) const
		// can throw exceptions
	{
		auto it = _resolver.find(guid);
		auto ite = _resolver.end();
		if(it != ite)
		{
			return it->second;
		}
		else
		{
			throw std::exception();
		}
	}

	ser::string get_guid(const ser::string& alias) const
		// can throw exceptions
	{
		for(auto& pair : _resolver)
		{
			if(pair.second == alias)
			{
				return pair.first;
			}
		}
		throw std::exception();
	}

	uint64 get_lag(const ser::string& guid) const
	{
		auto address = convert_to_address(guid);
		return _peer->GetClockDifferential(address);
	}

	void set_alias(const ser::string& alias)
	{
		_alias = alias;
		notify_alias(get_guid(), alias);
	}

	void print_resolver_table()
	{
		if(!_resolver.empty())
		{
			std::cout << "-- resolver table --" << std::endl;
			for(auto& pair : _resolver)
			{
				std::cout << pair.second << " (" << pair.first << ")" << std::endl;
			}
		}
		else
		{
			std::cout << "-- resolver table empty --" << std::endl;
		}
	}

protected:
	virtual void notify_alias(const ser::string& guid, const ser::string& alias) = 0;

public:
	// connection by client is failed (guid)
	fes::bind<ser::string> id_connection_attempt_failed;
	// connection by client is ok
	fes::bind<ser::string> on_connection_request_accepted;
	// other side is disconnected (client or server) (guid)
	fes::bind<ser::string> on_id_connection_lost;
	// a client request disconnection (guid)
	fes::bind<ser::string> on_id_disconnection_notification;
	// guid
	fes::bind<ser::string> on_new_client;
	fes::bind<ser::string> on_disconnect;
	// guid, alias
	fes::bind<ser::string, ser::string> on_changed_alias;
protected:
	RakNet::RakPeerInterface* _peer;
	std::unordered_map<int, ser::net_channel> _requests;
	std::unordered_map<std::string, ser::net_channel> _rooms;
	std::map<ser::string, ser::string> _resolver;
	ser::string _alias;
};

}

#endif

