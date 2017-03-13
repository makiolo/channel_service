#include "node.h"
//
#include <iostream>

using namespace RakNet;

namespace ser {

node::node(const ser::string& alias, uint16 port) : _server(alias, port)
{
	;
}

node::~node()
{
	;
}

}
