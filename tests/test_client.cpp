#include <iostream>
#include <sstream>
#include <cstdio>
#include <thread>
#include <map>
#include <fast-event-system/sync.h>
#include <fast-event-system/async_fast.h>
#include <teelogging/teelogging.h>
#include <asyncply/run.h>
#include <cppunix/parallel_scheduler.h>
#include "../serialize.h"
#include "../deserialize.h"
#include "../server.h"
#include "../client.h"
#include "../util.h"

#define CHAT 300

int main()
{
	cu::parallel_scheduler sch;

	// std::cout << std::unitbuf;
	// std::cout.sync_with_stdio(false);
	
	ser::client me("client", "127.0.0.1", 3333);
	std::cout << "client with guid: " << me.get_guid() << std::endl;

	me.get_channel(CHAT).connect(
		[&](int version, const ser::stream& pipe, const ser::string& guid) {
			if(me.get_guid() != guid)
			{
				ser::string data;
				ser::deserialize(pipe, data);
				std::cout << "<" << me.get_alias(guid) << "> " << data << std::endl;
			}
		}
	);

	bool quit = false;
	sch.spawn([&quit, &me](auto& yield){
		while(!quit)
		{
			me.update(yield);
		}
	});
	sch.spawn([&quit, &me](auto& yield)
	{
		std::map<std::string, ser::net_connection> conns;
		while(!quit)
		{
			std::string command = asyncply::await(yield, [&](){
				std::string command;
				std::getline(std::cin, command);
				return command; 
			});
			auto chunks = split(command, ' ');
			if(chunks[0] == "quit")
			{
				me.disconnect();
				quit = true;
			}
			else if(chunks[0] == "who")
			{
				std::cout << "I am " << me.get_alias() << " (" << me.get_guid() << ")" << std::endl;
				// estoy en los canales tal y pascual
				// tambien es necesario "who untercero" y saber en que canales esta
			}
			else if(chunks[0] == "query")
			{
				try
				{
					auto guid = me.get_guid(chunks[1].c_str());
					int i = 0;
					std::string buf = "";
					for(auto& word : chunks)
					{
						if(i > 1) buf += (word + ' ');
						++i;
					}
					me.one(guid, CHAT, 1, buf.c_str());
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
				me.topic(room, 1, buf.c_str());
			}
			else if(chunks[0] == "ping")
			{
				try
				{
					auto guid = me.get_guid(chunks[1].c_str());
					std::cout << "lag: " << me.get_lag(guid) << std::endl;
				}
				catch(std::exception& e)
				{
					std::cout << "user " << chunks[1].c_str() << " not found" << std::endl;
				}
			}
			else if(chunks[0] == "name")
			{
				me.set_alias(chunks[1].c_str());
			}
			else if(chunks[0] == "join")
			{
				auto r = chunks[1];
				// send msg to server
				me.join(r.c_str());
				// register callback
				// conns[r] = me.get_room(r).connect(
				me.get_room(r).connect(
					[&me, r](int version, const ser::stream& pipe, const ser::string& guid) {
						if(me.get_guid() != guid)
						{
							ser::string data;
							ser::deserialize(pipe, data);
							std::cout << "#" << r << " <" << me.get_alias(guid) << "> " << data << std::endl;
						}
					}
				);
			}
			else if(chunks[0] == "leave")
			{
				// send msg to server
				me.leave(chunks[1].c_str());
				// remove callback
				// conns.erase(chunks[1]);
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
				me.up(CHAT, 1, buf.c_str());
			}
			else if(chunks[0] == "status")
			{
				me.status();
			}
			else
			{
				me.all(CHAT, 1, command.c_str());
				me.up(CHAT, 1, command.c_str());
			}
			yield( cu::control_type{} );
		}
	});
	sch.run_forever();
	return 0;
}
