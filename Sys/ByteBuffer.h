#pragma once
#include <memory>
#include "Types.h"

namespace Andrew
{
	struct ByteBuffer
	{
		ByteBuffer() = default;

		ByteBuffer(u_long size)
			: size(size)
		{
			data = new char[size];
		}
		ByteBuffer(char* src, u_long size)
			: ByteBuffer(size)
		{
			memcpy(data, src, size);
		}
		~ByteBuffer()
		{
			delete[] data;
		}
		ByteBuffer(const ByteBuffer& other)
		{
			size = other.size;
			data = new char[size];
			memcpy(data, other.data, size);
		}
		ByteBuffer& operator=(const ByteBuffer& other)
		{
			if (this != &other)
			{
				size = other.size;
				data = new char[size];
				memcpy(data, other.data, size);
			}
			return *this;
		}
		u_long size = 0;
		char* data = nullptr;
	};
}