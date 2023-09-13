#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>


struct Command
{
    std::string text;
    std::chrono::system_clock::time_point timestamp;
};

class CommandProcessor
{
public:
    CommandProcessor(CommandProcessor* nextCommandProcessor = nullptr)
        : nextCommandProcessor(nextCommandProcessor)
    {
    }

    virtual ~CommandProcessor() = default;

    virtual void StartBlock() {}
    virtual void FinishBlock() {}

    virtual void ProcessCommand(const Command& command) = 0;

protected:
    CommandProcessor* nextCommandProcessor;
};

class ConsoleInput : public CommandProcessor
{
public:
    ConsoleInput(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
        , blockDepth(0)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        if (nextCommandProcessor)
        {
            if (command.text == "{")
            {
                if (blockDepth++ == 0)
                    nextCommandProcessor->StartBlock();
            }
            else if (command.text == "}")
            {
                if (--blockDepth == 0)
                    nextCommandProcessor->FinishBlock();
            }
            else
                nextCommandProcessor->ProcessCommand(command);
        }
    }

private:
    int blockDepth;
};

class ConsoleOutput : public CommandProcessor
{
public:
    ConsoleOutput(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        std::cout << command.text << std::endl;

        if (nextCommandProcessor)
            nextCommandProcessor->ProcessCommand(command);
    }
};

class ReportWriter : public CommandProcessor
{
public:
    ReportWriter(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        std::ofstream file(GetFilename(command), std::ofstream::out);
        file << command.text;

        if (nextCommandProcessor)
            nextCommandProcessor->ProcessCommand(command);
    }

private:
    std::string GetFilename(const Command& command)
    {
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        command.timestamp.time_since_epoch()).count();
        std::stringstream filename;
        filename << "bulk" << seconds << ".log";
        return filename.str();
    }
};

class BatchCommandProcessor : public CommandProcessor
{
public:
    BatchCommandProcessor(int bulkSize, CommandProcessor* nextCommandProcessor)
        : CommandProcessor(nextCommandProcessor)
        , bulkSize(bulkSize)
        , blockForced(false)
    {
    }

    ~BatchCommandProcessor()
    {
        if (!blockForced)
            DumpBatch();
    }

    void StartBlock() override
    {
        blockForced = true;
        DumpBatch();
    }

    void FinishBlock() override
    {
        blockForced = false;
        DumpBatch();
    }

    void ProcessCommand(const Command& command) override
    {
        commandBatch.push_back(command);

        if (!blockForced && commandBatch.size() >= bulkSize)
        {
            DumpBatch();
        }
    }
private:
    void ClearBatch()
    {
        commandBatch.clear();
    }

    void DumpBatch()
    {
        if (nextCommandProcessor && !commandBatch.empty())
        {
            std::string output = "bulk: " + Join(commandBatch);
            nextCommandProcessor->ProcessCommand(Command{output, commandBatch[0].timestamp});
        }
        ClearBatch();
    }

    static std::string Join(const std::vector<Command>& v)
    {
        std::stringstream ss;
        for(size_t i = 0; i < v.size(); ++i)
        {
            if(i != 0)
                ss << ", ";
            ss << v[i].text;
        }
        return ss.str();
    }
    int bulkSize;
    bool blockForced;
    std::vector<Command> commandBatch;
};

void RunBulk(int bulkSize)
{
    ReportWriter reportWriter;
    ConsoleOutput consoleOutput(&reportWriter);
    BatchCommandProcessor batchCommandProcessor(bulkSize, &consoleOutput);
    ConsoleInput consoleInput(&batchCommandProcessor);

    std::string text;
    while (std::getline(std::cin, text))
        consoleInput.ProcessCommand(Command{text, std::chrono::system_clock::now()});
}

int main(int argc, char const** argv)
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Bulk size is not specified." << std::endl;
            return 1;
        }

        int bulkSize = atoi(argv[1]);
        if (bulkSize == 0)
        {
            std::cerr << "Invalid bulk size." << std::endl;
            return 1;
        }

        RunBulk(bulkSize);
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
