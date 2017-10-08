#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "base.h"

namespace ser {

class serializer_API client : public network_base
{
public:
	explicit client(const ser::string& alias, const std::string& host, uint16 port, uint16 connection_timeout = DEFAULT_CONNECTION_TIMEOUT);
	virtual ~client();
	client(const client&) = delete;
	client& operator=(const client&) = delete;

	void status();
	void disconnect();
	virtual void on_package(RakNet::Packet* packet) override;

	template <typename ... Args>
	void one(const ser::string& to_guid, int port, int version, Args&& ... data) const
	{
		if(this->convert_to_guid(_server_guid) == to_guid)
		{
			up(port, version, std::forward<Args>(data)...);
		}
		else
		{
			RakNet::BitStream pipe;
			write_header(pipe, NET_MESSAGE_FORWARD);
			ser::serialize(pipe, this->get_guid(), to_guid, port, version, std::forward<Args>(data)...);
			_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
		}
	}

	template <typename ... Args>
	void all(int port, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_BROADCAST);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	template <typename ... Args>
	void topic(const ser::string& room, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_FORWARD);
		ser::serialize(pipe, this->get_guid(), room, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	template <typename ... Args>
	void up(int port, int version, Args&& ... data) const
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_DATA);
		ser::serialize(pipe, this->get_guid(), port, version, std::forward<Args>(data)...);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	void subscribe(const ser::string& room)
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_SUBSCRIBE);
		ser::serialize(pipe, this->get_guid(), room);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

	void unsubscribe(const ser::string& room)
	{
		RakNet::BitStream pipe;
		write_header(pipe, NET_MESSAGE_ROOM_UNSUBSCRIBE);
		ser::serialize(pipe, this->get_guid(), room);
		_peer->Send(&pipe, MEDIUM_PRIORITY, RELIABLE, 0, _server_guid, false);
	}

protected:
	virtual void notify_alias(const ser::string& guid, const ser::string& alias) override;
public:
	fes::bind<ser::string> on_disconnect_server;
protected:
	std::string _host;
	uint16 _port;
	uint16 _connection_timeout;
	RakNet::SystemAddress _server_guid;
};

} // end namespace Dune

#endif

