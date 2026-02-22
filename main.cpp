#include <iostream>
#include "core/Process/ProcessFinder.hpp"
#include "core/Process/ProcessReader.hpp"
#include "core/Process/ProcessScanner.hpp"

int main()
{
    std::cerr << "Запещенно \n";
    std::cerr << "Hello \n";
    ProcessScanner scan;
    ProcessReader read;
    ProcessFinder find(read, scan);
    std::string nameProcess;
    while (true)
    {
        std::cout << "Enter name process: \n";
        std::cin >> nameProcess;

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