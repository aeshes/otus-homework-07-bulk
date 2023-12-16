#pragma once

#include <iostream>
#include <fstream>

#include "command.h"

class command_subscriber
{
public:
	virtual ~command_subscriber() = default;

	virtual void receive(const command& cmd)
	{

	}

	virtual void relax() = 0;
};

class default_printer : public command_subscriber
{
public:
	void receive(const command& cmd) override
	{
		std::cout << (counter == 0 ? "bulk: " : ", ");
		std::cout << cmd.body();
	}

	void relax()
	{
		counter = 0;
		std::cout << std::endl;
	}

private:
	int counter = 0;
};

class file_printer : public command_subscriber
{
public:
	void receive(const command& cmd) override
	{
		if (counter == 0)
		{
			file.open("bulk" + std::to_string(cmd.time()) + ".log");
		}

		++counter;
		file << cmd.body() << std::endl;
	}

	void relax()
	{
		counter = 0;
		file.close();
	}

private:
	int counter = 0;
	std::ofstream file;
};