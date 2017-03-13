#ifndef NETWORKNODE_H
#define NETWORKNODE_H

#include "client.h"
#include "server.h"

namespace ser {

class serializer_API node
{
public:
	explicit node(const ser::string& alias, uint16 port);
	virtual ~node();
	node(const node&) = delete;
	node& operator=(const node&) = delete;
	
	void connect(const std::string& host, uint16 port)
	{
		_clients.emplace_back(_alias + "_client", host, port);
	}

	template <typename ... Args>
	void operator()(int port, int version, Args&& ... data) const
	{
		for(auto& client : _clients)
		{
			client.up(port, version, std::forward<Args>(data)...)
		}
	}
	
protected:
	ser::string _alias;
	std::vector<ser::client> _clients;
	ser::network_server _server;
};

} // end namespace Dune

#endif
