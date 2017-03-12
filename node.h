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

protected:
	ser::client _client;
	ser::network_server _server;
};

} // end namespace Dune

#endif

