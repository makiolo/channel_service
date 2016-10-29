#include "network_server.h"
//
#include <iostream>

namespace ser {

network_server::network_server(const ser::string& alias, uint16 port, uint16 max_clients, uint16 connection_timeout, uint16 reliable_timeout)
	: network_base(alias)
	, _port(port)
	, _max_clients(max_clients)
	, _reliable_timeout(reliable_timeout)
	, _connection_timeout(connection_timeout)
{
	RakNet::SocketDescriptor descriptor(_port, 0);
	_peer->Startup(max_clients, &descriptor, 1);
	_peer->SetMaximumIncomingConnections(_max_clients);
	_peer->SetOccasionalPing(_reliable_timeout > 0);
	_peer->SetUnreliableTimeout(_reliable_timeout);
	_peer->SetTimeoutTime(_connection_timeout, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	on_id_connection_lost.connect(on_disconnect_client);
	on_id_disconnection_notification.connect(on_disconnect_client);
	on_disconnect_client.connect([&](const ser::string& guid) {
		auto ite = _clients.end();
		auto it = _clients.find(guid.C_String());
		if(it != ite)
		{
			// propagate to clients
			{
				ser::stream pipe;
				write_header(pipe, NET_MESSAGE_DISCONNECT_CLIENT);
				ser::serialize(pipe, guid);
				_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			_resolver.erase(guid);
			_clients.erase(it);
		}
		cleanup();
	});
}

network_server::~network_server()
{

}

void network_server::disconnect()
{
	_clients.clear();
	cleanup();
}

void network_server::status()
{
	std::cout << "server in port " << _port << std::endl;
	if(!_clients.empty())
	{
		std::cout << "clients: " << std::endl;
		for(auto& pair : _clients)
		{
			pair.second->pprint(this);
		}
		std::cout << "-------------------------" << std::endl;

		std::cout << "rooms total " << _room_system.size() << std::endl;
		std::cout << "-------------------------" << std::endl;
		for(auto& room : _room_system)
		{
			std::cout << "room: " << room.second.get_name() << "" << std::endl;
			for(auto& weak_client : room.second)
			{
				if(auto client = weak_client.lock())
				{
					client->pprint(this);
				}
			}
		}
		std::cout << "server has " << _clients.size() << " clients. (max capacity is " << _max_clients << ")" << std::endl;
	}
	else
	{
		std::cout << "server is empty. (max capacity is " << _max_clients << ")"	<< std::endl;
	}
	// show resolver table
	print_resolver_table();
}

void network_server::on_new_incoming_connection(RakNet::Packet* packet)
{
	// new client
	auto packet_guid = this->convert_to_guid(packet->systemAddress);
	_clients[packet_guid.C_String()] = std::make_shared<ser::network_client_proxy>(_peer, packet->systemAddress);

	// me presento, soy servidor, el mismisimo
	{
		ser::stream repipe;
		write_header(repipe, NET_MESSAGE_DISCOVERY);
		ser::serialize(repipe, get_guid(), get_alias());
		_peer->Send(&repipe, MEDIUM_PRIORITY, RELIABLE, 0, packet->systemAddress, false);
	}

	// ordena a los demas clientes presentarse personalmente
	{
		ser::stream pipe;
		write_header(pipe, NET_MESSAGE_NEW_CLIENT);
		ser::serialize(pipe, packet_guid);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	// notify event
	on_new_client(packet_guid);
}

void network_server::on_package(RakNet::Packet* packet)
{
	switch (packet->data[0])
	{
		case NET_MESSAGE_DATA:
		{
			// mensaje de datos directo al server
			ser::string from_guid;
			int port;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, port, version, pipe);
			// publish data
			get_channel(port)(version, pipe, from_guid);
			break;
		}
		case NET_MESSAGE_BROADCAST:
		{
			// mensaje de cliente para ser retransmitido a todos
			ser::string from_guid;
			int port;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, port, version, pipe);

			// propagate to clients
			{
				ser::stream repipe;
				write_header(repipe, NET_MESSAGE_DATA);
				ser::serialize(repipe, from_guid, port, version, pipe);
				_peer->Send(&repipe, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			break;
		}
		case NET_MESSAGE_FORWARD:
		{
			// mensaje de cliente para ser entregado a otro cliente
			ser::string from_guid;
			ser::string to_guid;
			int port;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, to_guid, port, version, pipe);

			// propagate to client
			{
				ser::stream repipe;
				write_header(repipe, NET_MESSAGE_DATA);
				ser::serialize(repipe, from_guid, port, version, pipe);
				_peer->Send(&repipe, MEDIUM_PRIORITY, RELIABLE, 0, this->convert_to_address(to_guid), false);
			}
			break;
		}
		case NET_MESSAGE_DISCOVERY:
		{
			ser::string from_guid;
			ser::string alias;
			unpack(packet, from_guid, alias);

			notify_alias(from_guid, alias);
			break;
		}
		case NET_MESSAGE_ROOM_FORWARD:
		{
			// mensaje de cliente para ser entregado a los clientes de una sala
			ser::string from_guid;
			ser::string room;
			int version;
			ser::stream pipe;
			unpack(packet, from_guid, room, version, pipe);

			// comprobar que que from_guid esta en la sala room
			if(_room_system.get_room(room).guid_in_room(this, from_guid))
			{
				for(auto& weak_client : _room_system.get_room(room))
				{
					if(auto client = weak_client.lock())
					{
						ser::stream repipe;
						write_header(repipe, NET_MESSAGE_ROOM_DATA);
						ser::serialize(repipe, from_guid, room, version, pipe);
						_peer->Send(&repipe, MEDIUM_PRIORITY, RELIABLE, 0, client->get_raw(), false);
					}
				}
			}
			break;
		}
		case NET_MESSAGE_ROOM_JOIN:
		{
			ser::string from_guid;
			ser::string room;
			unpack(packet, from_guid, room);

			auto ite = _clients.end();
			auto it = _clients.find(from_guid.C_String());
			if(it != ite)
			{
				ser::network_room& r = _room_system.get_room(room);
				r.add_client( std::weak_ptr<ser::network_client_proxy>(it->second) );
			}
			else
			{
				throw std::exception();
			}
			break;
		}
		case NET_MESSAGE_ROOM_LEAVE:
		{
			ser::string from_guid;
			ser::string room;
			unpack(packet, from_guid, room);

			auto ite = _clients.end();
			auto it = _clients.find(from_guid.C_String());
			if(it != ite)
			{
				ser::network_room& r = _room_system.get_room(room);
				r.remove_client( std::weak_ptr<ser::network_client_proxy>(it->second) );
			}
			else
			{
				throw std::exception();
			}
			break;
		}
		default:
		{
			printf("<server> Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
}

void network_server::notify_alias(const ser::string& guid, const ser::string& alias)
{
	_resolver[guid] = alias;
	// notify event in local
	on_changed_alias(guid, alias);
	// propagate changes in clients
	{
		ser::stream pipe;
		write_header(pipe, NET_MESSAGE_DISCOVERY);
		ser::serialize(pipe, guid, alias);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
}

}

