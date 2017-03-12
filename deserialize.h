#ifndef _UNSERIALIZE_FUNCTOR_HPP
#define _UNSERIALIZE_FUNCTOR_HPP

// #include <msgpack.hpp>
#include <fast-event-system/metacommon/common.h>
#include "base.h"

namespace ser {

class serializer_API deserialize
{
public:
	template <typename ... Args>
	deserialize(const RakNet::BitStream& pipe, Args& ... args)
	{
		RakNet::BitStream pipe_copy(pipe.GetData(), pipe.GetNumberOfBytesUsed(), false);
		_unpack(pipe_copy, args...);
	}

	template <typename ... Args>
	deserialize(RakNet::BitStream& pipe, Args& ... args)
	{
		_unpack(pipe, args...);
	}

	template <typename T, typename ... Parms>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first, Parms& ... data) const
	{
		_apply<T>(pipe, std::forward<T>(first));
		_unpack(pipe, data...);
	}

	template <typename T>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first) const
	{
		_apply<T>(pipe, std::forward<T>(first));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, T&& first) const
	{
		pipe.Read(first);
	}
};

class serializer_API deserialize_delta
{
public:
	template <typename ... Args>
	deserialize_delta(const RakNet::BitStream& pipe, Args& ... args)
	{
		RakNet::BitStream pipe_copy(pipe.GetData(), pipe.GetNumberOfBytesUsed(), false);
		_unpack(pipe_copy, args...);
	}

	template <typename ... Args>
	deserialize_delta(RakNet::BitStream& pipe, Args& ... args)
	{
		_unpack(pipe, args...);
	}

	template <typename T, typename ... Parms>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first, Parms& ... data) const
	{
		_apply<T>(pipe, std::forward<T>(first));
		_unpack(pipe, data...);
	}

	template <typename T>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first) const
	{
		_apply<T>(pipe, std::forward<T>(first));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, T&& first) const
	{
		pipe.ReadDelta(first);
	}
};

class serializer_API deserialize_compressed
{
public:
	template <typename ... Args>
	deserialize_compressed(const RakNet::BitStream& pipe, Args& ... args)
	{
		RakNet::BitStream pipe_copy(pipe.GetData(), pipe.GetNumberOfBytesUsed(), false);
		_unpack(pipe_copy, args...);
	}

	template <typename ... Args>
	deserialize_compressed(RakNet::BitStream& pipe, Args& ... args)
	{
		_unpack(pipe, args...);
	}

	template <typename T, typename ... Parms>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first, Parms& ... data) const
	{
		_apply<T>(pipe, std::forward<T>(first));
		_unpack(pipe, data...);
	}

	template <typename T>
	constexpr void _unpack(RakNet::BitStream& pipe, T& first) const
	{
		_apply<T>(pipe, std::forward<T>(first));
	}

	template <typename T>
	constexpr void _apply(RakNet::BitStream& pipe, T&& first) const
	{
		pipe.ReadCompressed(first);
	}
};

}

#endif

