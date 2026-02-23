#include <iostream>
#include "core/Process/ProcessFinder.hpp"
#include "core/Process/ProcessReader.hpp"
#include "core/Process/ProcessScanner.hpp"
#include "core/Process/ModuleMapParser.hpp"

int main()
{
    std::cerr << "Запещенно \n";
    std::cerr << "Hello \n";
    ProcessScanner scan;
    ProcessReader read;
    ProcessFinder find(read, scan);
    ModuleMapParser moduleParser(read);
    pid_t pid = 0;

    std::string nameProcess;
    while (true)
    {
        std::cout << "Enter name process, 1 for ready: \n";
        std::cin >> nameProcess;
        
        if(nameProcess == "1")
        {
            std::cin >> pid;
            auto sModules = moduleParser.parse(pid);

            if(!sModules)
            {
                std::cerr << "Error parsing moduleMap process \n";
            }

            for(const auto& module : *sModules)
            {
                std::cout << "Start addr: " << module.start << "EndAddr: " << module.end << std::endl<< std::endl;
                std::cout << "Offsets: " << module.offset << "Permissions: " << module.permissions << std::endl<< std::endl;
                std::cout << "Path: " << module.pathname << std::endl<< std::endl;
            }
        }

        auto infoProc = find.searhProcessInfoByFilter(nameProcess);

        if(!infoProc)
        {
            std::cerr << "Not found or error \n";
            continue;
        }

        for(const auto& entry : *infoProc)
        {
            std::cout << "Найденые совпадения:   " << "Name:   " << entry.name << std::endl<< "Pid:   " << entry.pid << std::endl<< std::endl;
        }
    }
    return 0;
}