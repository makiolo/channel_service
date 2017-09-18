#ifndef NETWORKNODE_H
#define NETWORKNODE_H

#include "client.h"
#include "server.h"
#include "metacommon/common.h"

namespace ser {

/*
un nodo tipo MQTT acumula estado:
- un mapa (json), con clave (topic), tupla<Args>
- puede obedecer a ordenes externas de cambiar un estado
- cuando hay un cambio de estado, lo notifica por broadcast

// PC1 (192.168.1.100)
auto n1 = node<float, float>("producer humitura", 2000);
// notifica temperatura y humedad (por broadcast, lo reciben todos los que se han conectado)
node(25.0f, 67.0f);
node(25.5f, 66.5f);
node(26.f, 65.0f);

// PC2 (192.168.1.101)
auto n2 = node<float, float>("consumer humitura", 2000);
// subscribe conecta PC2 con PC1 mediante UDP
n2.subscribe("192.168.1.100", 2000);
// si tuvieramos un "tablon" con:
192.168.1.100:2000 -> "producer humitura"
192.168.1.101:2000 -> "consumer humitura"
// para hacer:
n2.subscribe("producer humitura");
for(auto& e in n2.range(yield))
{
	// unpack tuple
}
*/
	
template <typename ... Args>
class serializer_API node
{
public:
	explicit node(const ser::string& alias, uint16 port)
		: _alias(alias)
		, _server(alias, port)
	{
		_server.on_data.connect(
			[&](int port, int version, const ser::stream& pipe, const ser::string& guid) {
				// port
				// version
				// pipe to tuple<Args> -> to unpack Args ...
				// ser::deserialize(pipe, data...);


				// enviar la tupla a remoto
				// propagar en remoto
				/*
				for(auto& client : _clients)
				{
					client.up(port, version, std::forward<PARMS>(data)...)
				}
				*/
			
				// viene de llamadas remotas
				// meter en "_output" (truncando port o version ?)
			}
		);
	}
	
	node(const node&) = delete;
	node& operator=(const node&) = delete;

	// produccion nunca bloquea
	template <typename ... PARMS>
	void operator()(int port, int version, PARMS&&... data) const
	{
		// viene de llamadas locales
		// meter en "_output"
	}
	
	// blocking until a number of clients is connected ?
	/*
	void wait(int clients = 1)
	{
		
	}
	*/
	
	// consumo puede bloquear
	inline auto get() -> std::tuple<Args...>
	{
		auto& t = _output.get();
		// enviar la tupla a local
		// propagar en local
		
		return std::move(t);
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
	
protected:
	ser::string _alias;
	fes::async_fast<Args...> _output;
	std::vector<ser::client> _clients;
	ser::network_server _server;
};

} // end namespace Dune

#endif
