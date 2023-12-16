#pragma once

#include <string>
#include <ctime>

class command
{
public:
	command(std::time_t time, std::string& body)
		: timestamp(time), cmd(body)
	{

	}

	std::time_t time() const
	{
		return timestamp;
	}

	std::string body() const
	{
		return cmd;
	}

	bool isOpenScopeCommand() const
	{
		return cmd == "{";
	}

	bool isCloseScopeCommand() const
	{
		return cmd == "}";
	}

private:
	std::time_t timestamp;
	std::string cmd;
};