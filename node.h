#ifndef NETWORKNODE_H
#define NETWORKNODE_H

#include "client.h"
#include "server.h"

namespace ser {

template <typename ... Args>
class serializer_API node
{
public:
	explicit node(const ser::string& alias, uint16 port)
		: _alias(alias)
		, _server(alias, port)
	{
		_server.get_channel(0).connect(
			[&](int version, const ser::stream& pipe, const ser::string& guid) {
				// port
				// version
				// pipe to tuple<Args> -> to unpack Args ...
				/*
				std::cout << "version = " << version << std::endl;
				ser::deserialize<Args...>(pipe, data);
				std::cout << "data = " << data << std::endl;
				*/
			}
		);
	}

	virtual ~node()
	{
		;
	}
	
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
