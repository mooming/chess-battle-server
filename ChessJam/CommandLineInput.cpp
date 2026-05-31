#include "CommandLineInput.h"

#include <iostream>


CommandLineInput::CommandLineInput()
    : executionMode(ExecutionMode::None)
{
}

bool CommandLineInput::isValid() const
{
    return ExecutionMode::None <= executionMode && executionMode < ExecutionMode::Max;
}

void CommandLineInput::parse(int argc, const char* argv[])
{
    if (argc < 2)
    {
        executionMode = ExecutionMode::Monitor;
        monitorAddress = std::string("localhost");
        return;
    }


    parseCommand(argv[1]);

    if (executionMode <= ExecutionMode::None || executionMode >= ExecutionMode::Max)
    {

        return;
    }

    switch (executionMode)
    {
    default:
        std::cerr << "Parse Error: Invalid command index " << static_cast<int>(executionMode) << std::endl;
        executionMode = ExecutionMode::None;
        break;

    case ExecutionMode::Monitor:
        monitorAddress = std::string("localhost");
        break;

    case ExecutionMode::Match:
    case ExecutionMode::Judge:
    case ExecutionMode::GameServer:
    case ExecutionMode::GameClient:
        if (argc < 3)
        {
            monitorAddress = std::string("localhost");
        }
        else
        {
            monitorAddress = std::string(argv[2]);
        }

        break;
    }
}

ExecutionMode CommandLineInput::getMode() const { return executionMode; }

const std::string& CommandLineInput::getMonitorAddress() const { return monitorAddress; }

void CommandLineInput::print() const
{
    using namespace std;

    if (!isValid())
    {
        return;
    }

    auto& modes = ExecutionModeHelper::GetNames();
    cout << "Command: " << modes[static_cast<int>(executionMode)] << endl;
    cout << "Monitor Address: " << monitorAddress << endl;
}

void CommandLineInput::validate()
{
    if (isValid())
        return;

    executionMode = ExecutionMode::None;
}

void CommandLineInput::parseCommand(const char* str)
{
    int index = 0;

    auto& modes = ExecutionModeHelper::GetNames();
    for (auto& mode : modes)
    {
        if (mode == str)
        {
            executionMode = static_cast<ExecutionMode>(index);
            return;
        }

        ++index;
    }

    executionMode = ExecutionMode::None;
}
