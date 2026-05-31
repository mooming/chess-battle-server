#include <iostream>
#include <string>
#include <vector>

#include "CommandLineInput.h"
#include "ExecutionMode.h"
#include "StringHelper.h"
#include "Server.h"


namespace
{
    void printArguments(int argc, const char* argv[])
    {
        using namespace std;

        cout << "Arguments: " << endl;

        for (int i = 0; i < argc; ++i)
        {
            cout << "[" << i << ':' << argv[i] << "] " << endl;
        }

        cout << endl;
    }
} // Anonymous Namespace

int main(int argc, const char* argv[])
{
    using namespace std;

    printArguments(argc, argv);

    CommandLineInput commandInput;
    commandInput.parse(argc, argv);

    if (!commandInput.isValid())
    {
        string executed(argc > 0 ? argv[0] : "None");
        executed = StringHelper::extractFileName(executed);

        cout << "Usage)" <<endl;
        cout << "> " << executed << " Monitor" << endl;
        cout << "> " << executed << " Match <Monitor-Address>" << endl;
        cout << "> " << executed << " Judge <Monitor-Address>" << endl;
        cout << "> " << executed << " Server <Monitor-Address>" << endl;
        cout << "> " << executed << " Client <Monitor-Address>" << endl;

    }
    else {
        // Valid command, act based on mode
        switch (commandInput.getMode()) {
        case ExecutionMode::Monitor:
            // No server functionality for Monitor in this skeleton
            break;
        case ExecutionMode::Match:
        case ExecutionMode::Judge:
        case ExecutionMode::GameClient:
            // Placeholder for other roles
            break;
        case ExecutionMode::GameServer:
            {
                Server server;
                server.start(commandInput.getMonitorAddress());
            }
            break;
        default:
            break;
        }
    }

    commandInput.print();

    cout << endl << "Bye!" << endl;

    return 0;
}
