#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "network_base.h"

namespace ser {

class serializer_API network_client : public network_base
{
public:
	explicit network_client(const ser::string& alias, const std::string& host, uint16 port, uint16 connection_timeout = DEFAULT_CONNECTION_TIMEOUT);
    virtual ~network_client();
    network_client(const network_client&) = delete;
    network_client& operator=(const network_client&) = delete;

	void status();
	void disconnect();

	virtual void on_package(RakNet::Packet* packet) override;

	// send all
	template <typename ... Args>
	void broadcast(int port, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_BROADCAST);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	// send anybody
	template <typename ... Args>
	void send_room(const ser::string& room, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_FORWARD);
		ser::serialize(pipe, this->get_guid(), room, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	// send one
	template <typename ... Args>
	void send_private(const ser::string& to_guid, int port, int version, Args&& ... data) const
	{
		if(this->convert_to_guid(_server_guid) == to_guid)
		{
			send_parent(port, version, std::forward<Args>(data)...);
		}
		else
		{
			RakNet::BitStream pipe;
			write_header(pipe, NET_MESSAGE_FORWARD);
			ser::serialize(pipe, this->get_guid(), to_guid, port, version, std::forward<Args>(data)...);
			_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
		}
	}

	// send to server
	template <typename ... Args>
	void send_parent(int port, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_DATA);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	void join(const ser::string& room)
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_JOIN);
		ser::serialize(pipe, this->get_guid(), room);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	void leave(const ser::string& room)
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_LEAVE);
		ser::serialize(pipe, this->get_guid(), room);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

protected:
	virtual void notify_alias(const ser::string& guid, const ser::string& alias) override;
public:
	fes::sync<ser::string> on_disconnect_server;
protected:
	std::string _host;
	uint16 _port;
	uint16 _connection_timeout;
	RakNet::SystemAddress _server_guid;
};

} // end namespace Dune

#endif

