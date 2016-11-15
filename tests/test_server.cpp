#include <iostream>
#include <sstream>
#include <cstdio>
#include <thread>
#include <map>
#include <fes/sync.h>
#include <fes/async_fast.h>
#include <channel_service/serialize.h>
#include <channel_service/deserialize.h>
#include <channel_service/network_server.h>
#include <channel_service/network_client.h>
#include <channel_service/util.h>

#define CHAT 300

int main2()
{
	ser::network_server server("server", 5555);
	ser::network_client client("client", "127.0.0.1", 5555);
	client.on_connection_request_accepted.connect([&](auto guid_server) {
		std::cout << "conectado a " << guid_server << std::endl;
		client.send_parent(1, 2, 3);
	});
	server.get_channel(1).connect(
		[&](int version, const ser::stream& pipe, const ser::string& guid) {
			std::cout << "version = " << version << std::endl;
			int data;
			ser::deserialize(pipe, data);
			std::cout << "data = " << data << std::endl;
		}
	);

	std::thread t([&](){
		while(true)
		{
			server.dispatch_one();
			client.dispatch_one();
		}
	});
	t.join();
	return 0;
}

int main()
{
	std::cout << std::unitbuf;
	std::cout.sync_with_stdio(false);
	bool quit = false;
	ser::network_server server("server", 5555);
	std::cout << "server = " << server.get_guid() << std::endl;
	server.on_new_client.connect([&](auto guid) {
		std::cout << "joined client " << guid << std::endl;
	});
	server.on_changed_alias.connect([&](auto guid, auto alias) {
		std::cout << "client " << guid << " changed your alias to " << alias << std::endl;
	});
	server.on_disconnect_client.connect([&](auto guid) {
		std::cout << "disconnection client " << guid << std::endl;
	});
	server.on_disconnect.connect([&](auto guid) {
		std::cout << "disconnection any " << guid << std::endl;
	});

	// server.get_channel(600).connect(
	// 	[&](int version, const ser::stream& pipe, const ser::string& guid) {
	// 		int n, n2;
	// 		ser::string s, s2;
	// 		float f, f2;
	// 		ser::string cmd, cmd2;
	// 		int vel = 20;
	//
	// 		RakNet::BitStream pipe_copy(pipe.GetData(), pipe.GetNumberOfBytesUsed(), false);
	//
	// 		ser::deserialize(pipe_copy, n, s, f, cmd);
	// 		ser::deserialize_compressed(pipe_copy, n2, s2, f2, cmd2);
	// 		ser::deserialize_delta(pipe_copy, vel);
	// 		std::cout << "n = " << n << std::endl;
	// 		std::cout << "s = " << s << std::endl;
	// 		std::cout << "f = " << f << std::endl;
	// 		std::cout << "cmd = " << cmd << std::endl;
	// 		std::cout << "n2 = " << n2 << std::endl;
	// 		std::cout << "s2 = " << s2 << std::endl;
	// 		std::cout << "f2 = " << f2 << std::endl;
	// 		std::cout << "cmd2 = " << cmd2 << std::endl;
	// 		std::cout << "vel = " << vel << std::endl;
	// 	}
	// );
	server.get_channel(CHAT).connect(
		[&](int version, const ser::stream& pipe, const ser::string& guid) {
			if(server.get_guid() != guid)
			{
				ser::string data;
				ser::deserialize(pipe, data);
				std::cout << "<" << server.get_alias(guid) << "> " << data << std::endl;
			}
		}
	);

	std::thread t([&](){
		while(!quit)
		{
			server.dispatch_one();
		}
		server.dispatch_one();
	});
	while(!quit)
	{
		std::string command;
		std::getline(std::cin, command);
		if(!command.empty())
		{
			auto chunks = split(command, ' ');
			if(chunks[0] == "quit")
			{
				server.disconnect();
				quit = true;
			}
			else if(chunks[0] == "who")
			{
				std::cout << "I am " << server.get_alias() << " (" << server.get_guid() << ")" << std::endl;
			}
			else if(chunks[0] == "query")
			{
				try
				{
					auto guid = server.get_guid(chunks[1].c_str());
					int i = 0;
					std::string buf = "";
					for(auto& word : chunks)
					{
						if(i > 1) buf += (word + ' ');
						++i;
					}
					server.send_private(guid, CHAT, 1, buf.c_str());
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
				server.send_room(room, 1, buf.c_str());
			}
			else if(chunks[0] == "ping")
			{
				try
				{
					auto guid = server.get_guid(chunks[1].c_str());
					std::cout << "lag: " << server.get_lag(guid) << std::endl;
				}
				catch(std::exception& e)
				{
					std::cout << "user " << chunks[1].c_str() << " not found" << std::endl;
				}
			}
			else if(chunks[0] == "name")
			{
				server.set_alias(chunks[1].c_str());
			}
			else if(chunks[0] == "status")
			{
				server.status();
			}
			else
			{
				server.broadcast(CHAT, 1, command.c_str());
			}
		}
	}
	t.join();
	return 0;
}

