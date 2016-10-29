#ifndef NETWORKNODE_H
#define NETWORKNODE_H

#include "network_client.h"
#include "network_server.h"

namespace ser {

class serializer_API network_node
{
public:
	explicit network_node(const ser::string& alias, uint16 port);
    virtual ~network_node();
    network_node(const network_node&) = delete;
    network_node& operator=(const network_node&) = delete;

protected:
	ser::network_client _client;
	ser::network_server _server;
};

} // end namespace Dune

#endif

