#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <memory>
#include <set>
#include "base.h"

namespace ser {

class serializer_API network_client_proxy
{
public:
	explicit network_client_proxy(RakNet::RakPeerInterface* peer, const RakNet::SystemAddress& address)
		: _peer(peer)
		, _address(address)
	{ ; }
	~network_client_proxy()
	{
		_peer->CloseConnection(_address, true);
	}
	network_client_proxy(const network_client_proxy&) = delete;
	network_client_proxy& operator=(const network_client_proxy&) = delete;

	ser::string get_guid(ser::network_base* base) const
	{
		return base->convert_to_guid(get_raw());
	}

	void pprint(ser::network_base* base)
	{
		auto packet_guid = get_guid(base);
		ser::string alias;
		try
		{
			alias = base->get_alias(packet_guid);
		}
		catch(std::exception& e)
		{
			alias = "unknown";
		}
		std::cout << "\t" << packet_guid << " (" << alias << ") - " << get_raw().ToString() << std::endl;
	}

	const RakNet::SystemAddress& get_raw() const {return _address;}
protected:
	RakNet::RakPeerInterface* _peer;
	RakNet::SystemAddress _address;
};

class serializer_API network_room
{
public:
	using weak_client_type = std::weak_ptr<ser::network_client_proxy>;

	explicit network_room() : _name(ROOM_DEFAULT) { ; }
	network_room(const network_room&) = delete;
	network_room& operator=(const network_room&) = delete;

	void set_name(const ser::string& name)
	{
		_name = name;
	}

	const ser::string& get_name() const {return _name;}

	void add_client(const weak_client_type& client)
	{
		_clients.emplace(client);
	}

	void remove_client(const weak_client_type& client)
	{
		_clients.erase(client);
	}

	/*
	 * remove disconnected clients
	 */
	void cleanup()
	{
		auto it = _clients.begin();
		auto ite = _clients.end();
		for(; it != ite;)
		{
			if((*it).expired())
			{
				it = _clients.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	bool guid_in_room(ser::network_base* base, const ser::string& guid) const
	{
		for(auto& _client : _clients)
		{
			if(auto client = _client.lock())
			{
				if(client->get_guid(base) == guid)
				{
					return true;
				}
			}
		}
		return false;
	}

	auto begin() const
	{
		return _clients.begin();
	}

	auto end() const
	{
		return _clients.end();
	}

	auto begin()
	{
		return _clients.begin();
	}

	auto end()
	{
		return _clients.end();
	}

	size_t size() const
	{
		return _clients.size();
	}

	bool empty() const
	{
		return _clients.empty();
	}
protected:
	ser::string _name;
	std::set<weak_client_type, std::owner_less<weak_client_type> > _clients;
};

class serializer_API network_room_system
{
public:
	network_room_system(const network_room_system&) = delete;
	network_room_system& operator=(const network_room_system&) = delete;

	ser::network_room& get_room(const ser::string& name = ROOM_DEFAULT)
	{
		auto& room = _rooms[name];
		room.set_name(name);
		return room;
	}

	/*
	 * remove disconnected clients
	 */
	void cleanup()
	{
		for(auto& pair : _rooms)
		{
			pair.second.cleanup();
		}

		// despues del cleanup, borrar habitaciones vacias
		auto it = _rooms.begin();
		auto ite = _rooms.end();
		for(; it != ite;)
		{
			// borrar rooms vacias
			if(it->second.empty())
			{
				it = _rooms.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	auto begin()
	{
		return _rooms.begin();
	}

	auto end()
	{
		return _rooms.end();
	}

	size_t size() const
	{
		return _rooms.size();
	}

	bool empty() const
	{
		return _rooms.empty();
	}

protected:
	std::map<ser::string, ser::network_room> _rooms;
};

class serializer_API network_server : public network_base
{
public:
	using shared_client_type = std::shared_ptr<ser::network_client_proxy>;

	explicit network_server(const ser::string& alias, uint16 port, uint16 max_clients = MAX_CLIENTS_DEFAULT, uint16 connection_timeout = DEFAULT_CONNECTION_TIMEOUT, uint16 reliable_timeout = DEFAULT_RELIABLE_TIMEOUT);
	virtual ~network_server();
	network_server(const network_server&) = delete;
	network_server& operator=(const network_server&) = delete;

	void status();
	void disconnect();

	/*
	 * remove disconnected clients
	 */
	void cleanup()
	{
		_room_system.cleanup();
	}

	virtual void on_new_incoming_connection(RakNet::Packet* packet) override;
	virtual void on_package(RakNet::Packet* packet) override;

	// send all
	template <typename ... Args>
	void broadcast(int port, int version, Args&& ... data) const
	{
		ser::stream pipe;
		write_header(pipe, NET_MESSAGE_DATA);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	// send anybody
	template <typename ... Args>
	void send_room(const ser::string& room, int version, Args&& ... data)
	{
		// is non-const, can create new rooms
		for(auto& weak_client : _room_system.get_room(room))
		{
			if(auto client = weak_client.lock())
			{
				ser::stream pipe;
				write_header(pipe, NET_MESSAGE_ROOM_DATA);
				ser::serialize(pipe, this->get_guid(), room, version, std::forward<Args>(data)...);
				_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, client->get_raw(), false);
			}
		}
	}

	// send one
	template <typename ... Args>
	void send_private(const ser::string& to_guid, int port, int version, Args&& ... data) const
	{
		ser::stream pipe;
		write_header(pipe, NET_MESSAGE_DATA);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, this->convert_to_address(to_guid), false);
	}

public:
	fes::bind<ser::string> on_disconnect_client;
protected:
	virtual void notify_alias(const ser::string& guid, const ser::string& alias) override;

protected:
	uint16 _port;
	uint16 _max_clients;
	uint16 _connection_timeout;
	uint16 _reliable_timeout;
	std::map<std::string, shared_client_type> _clients;
	ser::network_room_system _room_system;
};

} // end namespace Dune

#endif
