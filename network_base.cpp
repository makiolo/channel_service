#include "network_base.h"
//
#include <unistd.h>

using namespace RakNet;

namespace ser {

network_base::network_base(const ser::string& alias)
	: _alias(alias)
{
	_peer = RakPeerInterface::GetInstance();
}

network_base::~network_base()
{
	_peer->Shutdown(0);
}

bool network_base::dispatch_one()
{
	if(Packet* packet = _peer->Receive())
	{
		if(packet)
		{
			switch(packet->data[0])
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				{
					printf("Another client has disconnected.\n");
				}
				break;
			case ID_REMOTE_CONNECTION_LOST:
				{
					printf("Another client has lost the connection.\n");
				}
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				{
					printf("Another client has connected.\n");
				}
				break;
			case ID_NEW_INCOMING_CONNECTION:
				{
					on_new_incoming_connection(packet);
				}
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				{
					printf("The server is full.\n");
				}
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				{
					// printf("disconnection notified: %s\n", packet->guid.ToString());
					// auto guid = convert_to_guid(packet->systemAddress);
					// printf("disconnection notified guid: %s\n", guid.C_String());
					on_id_disconnection_notification(packet->guid.ToString());
				}
				break;
			case ID_CONNECTION_LOST:
				{
					// printf("connection lost: %s\n", packet->guid.ToString());
					// auto guid = convert_to_guid(packet->systemAddress);
					// printf("connection lost guid: %s\n", guid.C_String());
					on_id_connection_lost(packet->guid.ToString());
				}
				break;
			case ID_ALREADY_CONNECTED:
				{
					printf("ID_ALREADY_CONNECTED with GUID: %s\n", packet->guid.ToString());
				}
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				{
					printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				}
				break;
			case ID_CONNECTION_BANNED:
				{
					printf("We are banned from this server.\n");
				}
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				{
					auto guid = convert_to_guid(packet->systemAddress);
					id_connection_attempt_failed(guid);
				}
				break;
			case ID_INVALID_PASSWORD:
				{
					printf("ID_INVALID_PASSWORD\n");
				}
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					on_connection_request_accepted(packet->guid.ToString());
				}
				break;
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				{
					printf("Ping from %s\n", packet->systemAddress.ToString(true));
				}
				break;
			default:
				{
					on_package(packet);
				}
				break;
			}
			_peer->DeallocatePacket(packet);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

} // end namespace Dune

