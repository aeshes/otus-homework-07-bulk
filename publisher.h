#pragma once

#include <vector>
#include <algorithm>
#include <string>

#include "subscriber.h"
#include "command.h"

class command_publisher
{
public:
	virtual ~command_publisher() = default;

	virtual void addSubscriber(command_subscriber* subscriber) = 0;
};

class base_publisher : public command_publisher
{
public:
	void addSubscriber(command_subscriber* subscriber)
	{
		subscribers.push_back(subscriber);
	}

protected:
	void publish(const command& cmd)
	{
		for (auto sub : subscribers)
		{
			sub->receive(cmd);
		}
	}

	void notifyRelax()
	{
		std::for_each(subscribers.cbegin(), subscribers.cend(), [](auto sub) {
			sub->relax();
		});
	}

private:
	std::vector<command_subscriber*> subscribers;
};

class command_reader : public base_publisher
{
public:
	void run()
	{
		std::string cmd;
		while (std::getline(std::cin, cmd))
		{
			if (!cmd.empty())
			{
				publish(command(std::time(nullptr), cmd));
			}
		}
		notifyRelax();
	}
};

class accumulator : public command_subscriber, public base_publisher
{
public:
	accumulator(int bulkSize)
		: bulkSize(bulkSize)
	{
		commands.reserve(bulkSize);
	}

	void receive(const command& cmd) override
	{
		if (cmd.isOpenScopeCommand())
		{
			if (scopeLevel != 0)
			{
				flushAccumulatedCommands();
			}
			++scopeLevel;
		}
		else if (cmd.isCloseScopeCommand() && (scopeLevel != 0))
		{
			--scopeLevel;
			if (scopeLevel == 0)
			{
				flushAccumulatedCommands();
			}
		}
		else
		{
			commands.push_back(cmd);
			if ((scopeLevel == 0) && commands.size() == bulkSize)
			{
				flushAccumulatedCommands();
			}
		}
	}

	void relax() override
	{
		if (scopeLevel == 0)
		{
			flushAccumulatedCommands();
		}
	}

private:
	void flushAccumulatedCommands()
	{
		for (const command& cmd : commands)
		{
			publish(cmd);
		}
		notifyRelax();
		commands.clear();
	}

private:
	std::vector<command> commands;
	int bulkSize = 0;
	int scopeLevel = 0;
};