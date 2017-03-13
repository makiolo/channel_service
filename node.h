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

	template <typename ... PARMS>
	void operator()(int port, int version, PARMS&&... data) const
	{
		for(auto& client : _clients)
		{
			client.up(port, version, std::forward<PARMS>(data)...)
		}
	}
	
	void connect(const std::string& host, uint16 port)
	{
		_clients.emplace_back(_alias + "_client", host, port);
	}

	/*
	template <typename T, typename ... ARGS>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const ARGS&...))
	{
		return _output.connect(obj, ptr_func);
	}

	template <typename METHOD>
	inline weak_connection<Args...> connect(METHOD&& method)
	{
		return _output.connect(std::forward<METHOD>(method));
	}
	
	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](Args... data)
			{
				callback(std::move(data)...);
			});
	}

	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](Args... data)
			{
				queue(std::move(data)...);
			});
	}

	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([priority, delay, &queue](Args... data)
			{
				queue(priority, delay, std::move(data)...);
			});
	}
	*/
	
	inline auto get() -> std::tuple<Args...>
	{
		return _get();
	}
	
protected:
	template <typename Tuple, int... S>
	inline void get(Tuple&& top, seq<S...>) const
	{
		_output(std::get<S>(std::forward<Tuple>(top))...);
	}

	inline auto _get() -> std::tuple<Args...>
	{
		// esperar a tener algo en el server
		std::tuple<Args...> t;
		// leer el primer mensaje
		get(std::forward<std::tuple<Args...> >(t), gens<sizeof...(Args)>{});
		return std::move(t);
	}
	
protected:
	sync<Args...> _output;
	ser::string _alias;
	std::vector<ser::client> _clients;
	ser::network_server _server;
};

} // end namespace Dune

#endif
