#pragma once
#include <iostream>
#include <string>
#include "ByteBuffer.h"

namespace Andrew
{
	class UserIO
	{
	public:
		UserIO(u_long size) : buffer(size)
		{
		}
		void showOptions(bool auth)
		{
			std::cout << "\n==== Options: ====\n";
			if (auth)
			{
				std::cout	<< "[1] - send a picture\n"
							<< "[2] - exit\n";
			}
			else
			{
				std::cout	<< "[1] - auth\n"
							<< "[2] - exit\n";
			}
		}
		void getInput()
		{
			std::cin.getline(buffer.data, buffer.size);
		}
		void write(const char* str)
		{
			std::cout << str;
		}
		void writeln(const char& str)
		{
			std::cout << str << '\n';
		}
		void write(const std::string& str)
		{
			std::cout << str;
		}
		void writeln(const std::string& str)
		{
			std::cout << str << '\n';
		}
		int getOption(int rangeStart, int rangeEnd)
		{
			getInput();
			return extractOption(rangeStart, rangeEnd);
		}
		int askForInt(const char* message)
		{
			write(message);
			getInput();
			int id = inputToInt();
			write("\n");
			return id;
		}
		void askForString(const char* message, std::string& dst)
		{
			write(message);
			getInput();
			dst = std::string(buffer.data);
			write("\n");
		}
		const ByteBuffer& getBuffer() const { return buffer; }
	private:
		ByteBuffer buffer;
	private:
		/** returns 0 if failed
		*	[rangeStart, rangeEnd)
		*/
		int extractOption(int rangeStart, int rangeEnd)
		{
			int option = inputToInt();
			if (option >= rangeEnd || option < rangeStart)
				return 0;
			return option;
		}
		int inputToInt()
		{
			return atoi(buffer.data);
		}
	};
}
