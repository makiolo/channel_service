#include "node.h"
//
#include <iostream>

using namespace RakNet;

namespace ser {

node::node(const ser::string& alias, uint16 port)
			: _server(alias + "_server", port)
			, _client(alias + "_client", "localhost", port)
{
	;
}

node::~node()
{
	;
}

}

