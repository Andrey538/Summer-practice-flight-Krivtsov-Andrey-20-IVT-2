#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "ByteBuffer.h"

namespace Andrew
{
	class File
	{
	public:
		File() = default;
		File(const ByteBuffer& buf)
		{
			buffer = buf;
		}
		void read(const std::string& path)
		{
			std::ifstream in(path, std::ios::in);
			mRead(path, in);
			in.close();
		}
		void write(const std::string& path)
		{
			std::ofstream out(path, std::ios::out);
			mWrite(path, out);
			out.close();
		}
		void bRead(const std::string& path)
		{
			std::ifstream in(path, std::ios::in | std::ios::binary);
			mRead(path, in);
			in.close();
		}

		void bWrite(const std::string& path)
		{
			std::ofstream out(path, std::ios::out | std::ios::binary);
			mWrite(path, out);
			out.close();
		}
		const char* getBuffer() const { return buffer.data; }
		int getSize() const { return buffer.size; }
	protected:
		void mRead(const std::string& path, std::ifstream& in)
		{
			if (!in)
			{
				std::cout << "Can't open the file, path: " << path << '\n';
				return;
			}
			in.seekg(0, std::ios::end);
			int size = in.tellg();
			if (size == -1)
			{
				std::cout << "Can't read the file, path: " << path << '\n';
				return;
			}
			in.seekg(0, std::ios::beg);
			buffer = ByteBuffer(size);
			in.read(buffer.data, size);
		}
		void mWrite(const std::string& path, std::ofstream& out)
		{
			out.write(buffer.data, buffer.size);
		}
	private:
		ByteBuffer buffer;
	};
}