#include <iostream>
#include <sstream>
#include <cstdio>
#include <thread>
#include <map>
#include <fast-event-system/sync.h>
#include <fast-event-system/async_fast.h>
#include <teelogging/teelogging.h>
#include "../serialize.h"
#include "../deserialize.h"
#include "../network_server.h"
#include "../network_client.h"
#include "../util.h"

#define CHAT 300

int main()
{
	std::cout << std::unitbuf;
	std::cout.sync_with_stdio(false);
	bool quit = false;
	ser::network_client client("client", "127.0.0.1", 5555);
	std::cout << "client = " << client.get_guid() << std::endl;

	client.on_connection_request_accepted.connect([&](auto guid_server) {
		std::cout << "connected to " << guid_server << std::endl;
	});
	// si el cliente falla la conexion
	client.id_connection_attempt_failed.connect([&](auto packet) {
		LOGE("connection with server failed");
		exit(1);
	});
	// un nuevo cliente se ha conectado al servidor
	client.on_new_client.connect([&](auto new_guid) {
		std::cout << "joined client " << new_guid << std::endl;
	});
	// otro cliente se ha cambiado de nombre
	client.on_changed_alias.connect([&](auto guid, auto alias) {
		std::cout << "client " << guid << " changed your alias to " << alias << std::endl;
	});
	// si se ha desconectado un cliente hermano
	client.on_disconnect.connect([&](auto guid) {
		LOGI("disconnection of client %s", guid.C_String());
	});
	// si se pierde la conexion con el servidor
	client.on_disconnect_server.connect([&](auto guid) {
		LOGE("connection lost with server %s", guid.C_String());
		exit(1);
	});

	client.get_channel(CHAT).connect(
		[&](int version, const ser::stream& pipe, const ser::string& guid) {
			if(client.get_guid() != guid)
			{
				ser::string data;
				ser::deserialize(pipe, data);
				std::cout << "<" << client.get_alias(guid) << "> " << data << std::endl;
			}
		}
	);

	std::map<std::string, ser::net_connection> conns;

	std::thread t([&]() {
		while(!quit)
		{
			client.update();
		}
	});
	while(!quit)
	{
		std::string command;
		std::getline(std::cin, command);
		auto chunks = split(command, ' ');
		if(chunks[0] == "quit")
		{
			client.disconnect();
		}
		else if(chunks[0] == "who")
		{
			std::cout << "I am " << client.get_alias() << " (" << client.get_guid() << ")" << std::endl;
			// estoy en los canales tal y pascual
			// tambien es necesario "who untercero" y saber en que canales esta
		}
		else if(chunks[0] == "query")
		{
			try
			{
				auto guid = client.get_guid(chunks[1].c_str());
				int i = 0;
				std::string buf = "";
				for(auto& word : chunks)
				{
					if(i > 1) buf += (word + ' ');
					++i;
				}
				client.send_private(guid, CHAT, 1, buf.c_str());
			}
			catch(std::exception& e)
			{
				std::cout << "user " << chunks[1].c_str() << " not found" << std::endl;
			}
		}
		else if(chunks[0] == "room")
		{
			auto room = chunks[1].c_str();
			int i = 0;
			std::string buf = "";
			for(auto& word : chunks)
			{
				if(i > 1) buf += (word + ' ');
				++i;
			}
			client.send_room(room, 1, buf.c_str());
		}
		else if(chunks[0] == "ping")
		{
			try
			{
				auto guid = client.get_guid(chunks[1].c_str());
				std::cout << "lag: " << client.get_lag(guid) << std::endl;
			}
			catch(std::exception& e)
			{
				std::cout << "user " << chunks[1].c_str() << " not found" << std::endl;
			}
		}
		else if(chunks[0] == "name")
		{
			client.set_alias(chunks[1].c_str());
		}
		else if(chunks[0] == "join")
		{
			auto r = chunks[1];
			// send msg to server
			client.join(r.c_str());
			// register callback
			conns[r] = client.get_room(r).connect(
				[&client, r](int version, const ser::stream& pipe, const ser::string& guid) {
					if(client.get_guid() != guid)
					{
						ser::string data;
						ser::deserialize(pipe, data);
						std::cout << "#" << r << " <" << client.get_alias(guid) << "> " << data << std::endl;
					}
				}
			);
		}
		else if(chunks[0] == "leave")
		{
			// send msg to server
			client.leave(chunks[1].c_str());
			// remove callback
			conns.erase(chunks[1]);
		}
		else if(chunks[0] == "server")
		{
			int i = 0;
			std::string buf = "";
			for(auto& word : chunks)
			{
				if(i > 0) buf += (word + ' ');
				++i;
			}
			client.send_parent(CHAT, 1, buf.c_str());
		}
		else if(chunks[0] == "status")
		{
			client.status();
		}
		else
		{
			// ser::stream pipe;
			// ser::serialize(pipe, 1, "hola", 56.0f, command.c_str());
			// ser::serialize_compressed(pipe, 1, "hola", 0.567891234f, command.c_str());
			// ser::serialize_delta(pipe, std::make_pair(40, 20));
			// client.send_parent(600, 1, pipe);

			client.broadcast(CHAT, 1, command.c_str());
			client.send_parent(CHAT, 1, command.c_str());
		}
	}
	t.join();
	return 0;
}

