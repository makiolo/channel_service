#include "network_node.h"
//
#include <iostream>

using namespace RakNet;

namespace ser {

network_node::network_node(const ser::string& alias, uint16 port)
			: _server(alias + "_server", port)
			, _client(alias + "_client", "localhost", port)
{
	;
}

network_node::~network_node()
{
	;
}

}

