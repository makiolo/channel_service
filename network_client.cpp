#include "network_client.h"
//
#include <iostream>

namespace ser {

network_client::network_client(const ser::string& alias, const std::string& host, uint16 port, uint16 connection_timeout)
			: network_base(alias)
			, _host(host)
			, _port(port)
			, _connection_timeout(connection_timeout)
{
	RakNet::SocketDescriptor descriptor;
	_peer->Startup(1, &descriptor, 1);
	_peer->Connect(_host.c_str(), _port, 0, 0);

	on_id_connection_lost.connect(on_disconnect_server);
	on_id_disconnection_notification.connect(on_disconnect_server);
	on_disconnect_server.connect([&](const ser::string& guid) {
		_resolver.erase(guid);
	});
	on_connection_request_accepted.connect([&](const ser::string& server_guid) {
		_server_guid = this->convert_to_address(server_guid);
		_peer->SetTimeoutTime(_connection_timeout, _server_guid);
	});
}

network_client::~network_client()
{

}

void network_client::status()
{
	std::cout << "client connected to " << _server_guid.ToString() << std::endl;
	// show resolver table
	print_resolver_table();
}

void network_client::disconnect()
{
	_peer->CloseConnection(_server_guid, true);
}

void network_client::on_package(RakNet::Packet* packet)
{
	switch (packet->data[0])
	{
		case NET_MESSAGE_DATA:
		{
			// mensaje de datos
			ser::string from_guid;
			int port;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, port, version, pipe);
			get_channel(port)(version, pipe, from_guid);
			break;
		}
		case NET_MESSAGE_ROOM_DATA:
		{
			// mensaje de room
			ser::string from_guid;
			ser::string room;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, room, version, pipe);
			get_room(std::string(room.C_String()))(version, pipe, from_guid);
			break;
		}
		case NET_MESSAGE_NEW_CLIENT:
		{
			// el servidor nos pide presentarnos a un nuevo cliente
			ser::string new_guid;
			unpack(packet, new_guid);

			// Hola new guid, mi alias es xxx
			{
				ser::stream pipe;
				write_header(pipe, NET_MESSAGE_DISCOVERY);
				ser::serialize(pipe, get_guid(), get_alias());
				_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
			}

			// notify new client
			on_new_client(new_guid);
			break;
		}
		case NET_MESSAGE_DISCOVERY:
		{
			// es la presentación que contiene el alias de un nuevo cliente
			ser::string from_guid;
			ser::string alias;
			unpack(packet, from_guid, alias);

			// register in server
			_resolver[from_guid] = alias;

			// notify event
			on_changed_alias(from_guid, alias);
			break;
		}
		case NET_MESSAGE_DISCONNECT_CLIENT:
		{
			ser::string guid;
			unpack(packet, guid);

			// propagate event
			on_disconnect(guid);
			// remove resolver
			_resolver.erase(guid);
			break;
		}
		default:
		{
			printf("<client> Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
}

void network_client::notify_alias(const ser::string& guid, const ser::string& alias)
{
	// notify change to server
	{
		ser::stream pipe;
		write_header(pipe, NET_MESSAGE_DISCOVERY);
		ser::serialize(pipe, guid, alias);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}
}

}

