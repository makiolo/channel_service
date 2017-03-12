#ifndef _SERIALIZE_FUNCTOR_HPP
#define _SERIALIZE_FUNCTOR_HPP

#include <iostream>
#include <tuple>
#include <string>
#include "api.h"
#include "base.h"
#include <type_traits>

namespace ser {

class serializer_API serialize
{
public:
	template <typename ... Args>
	inline serialize(RakNet::BitStream& pipe, Args&& ... args)
	{
		this->_pack(pipe, std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr void _pack(RakNet::BitStream& pipe, T&& first, Args&& ... data) const
	{
		_apply<T>(pipe, std::forward<T>(first));
		this->_pack(pipe, std::forward<Args>(data)...);
	}

	template <typename T>
	constexpr void _pack(RakNet::BitStream& pipe, T&& first) const
	{
		_apply<T>(pipe, std::forward<T>(first));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, T&& first) const
	{
		pipe.Write(std::forward<T>(first));
	}
};

class serializer_API serialize_delta
{
public:
	template <typename ... Args>
	inline serialize_delta(RakNet::BitStream& pipe, Args&& ... args)
	{
		this->_pack(pipe, std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr void _pack(RakNet::BitStream& pipe, std::pair<T, T>&& p, Args&& ... data) const
	{
		_apply<T>(pipe, std::forward<std::pair<T, T>>(p));
		this->_pack(pipe, std::forward<Args>(data)...);
	}

	template <typename T>
	constexpr void _pack(RakNet::BitStream& pipe, std::pair<T, T>&& p) const
	{
		_apply<T>(pipe, std::forward<std::pair<T, T>>(p));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, std::pair<T, T>&& p) const
	{
		pipe.WriteDelta(std::forward<T>(p.first), std::forward<T>(p.second));
	}
};

class serializer_API serialize_compressed
{
public:
	template <typename ... Args>
	inline serialize_compressed(RakNet::BitStream& pipe, Args&& ... args)
	{
		this->_pack(pipe, std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr void _pack(RakNet::BitStream& pipe, T&& first, Args&& ... data) const
	{
		_apply<T>(pipe, std::forward<T>(first));
		this->_pack(pipe, std::forward<Args>(data)...);
	}

	template <typename T>
	constexpr void _pack(RakNet::BitStream& pipe, T&& first) const
	{
		_apply<T>(pipe, std::forward<T>(first));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, T&& first) const
	{
		pipe.WriteCompressed(std::forward<T>(first));
	}
};

}

#endif

