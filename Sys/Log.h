#pragma once
#include <iostream>
#include <fstream>
#include <string>

namespace Andrew
{
	class Log
	{
	public:
		Log(const std::string& path)
			: path(path)
		{
		}
		void write(const char* buffer, uint64_t size)
		{
			out.open(path, std::ios::app | std::ios::out);
			std::cout << buffer;
			out.write(buffer, size);
			out.close();
		}
		void write(const std::string& buffer)
		{
			write(buffer.c_str(), buffer.size());
		}
	private:
		std::string path;
		std::fstream out;
	};
}