#include <iostream>
#include "core/Process/ProcessFinder.hpp"
#include "core/Process/ProcessReader.hpp"
#include "core/Process/ProcessScanner.hpp"
#include "core/Process/ModuleMapParser.hpp"
#include "core/Process/ModuleFilter.hpp"
#include "core/Scanner/scanner.hpp"
#include "core/Scanner/value.hpp"
#include <iomanip>

int main()
{
    std::cerr << "Debug\n";

    ProcessScanner scanner;
    ProcessReader reader;
    ProcessFinder finder(reader, scanner);
    ModuleMapParser moduleParser(reader);
    auto filter = std::make_shared<ModuleFilter>();

    ModuleFilterConfig config;
    ScanerContext scanCfg{};

    pid_t pid = 0;
    std::string nameProcess;
    

    config.onlyWritable = true;
    config.onlyExecutable = false;
    config.excludeSystemLibs = true;
    config.includeAnonymous = true;
    config.includeDrivers = false;
    config.includeTemporaryFile = false;

    std::cout << "[Info] Filter settings:\n";
    std::cout << "  onlyWritable: " << config.onlyWritable << "\n";
    std::cout << "  onlyExecutable: " << config.onlyExecutable << "\n";
    std::cout << "  excludeSystemLibs: " << config.excludeSystemLibs << "\n";
    std::cout << "  includeAnonymous: " << config.includeAnonymous << "\n";

    while (true)
    {
        std::cout << "\nEnter process name (or '1' to enter PID directly, 'q' to quit): ";
        std::cin >> nameProcess;
        if (nameProcess == "q") break;

        std::vector<MemoryRegion> modules;

        if (nameProcess == "1")
        {
            std::cout << "Enter PID: ";
            std::cin >> pid;

            auto sModules = moduleParser.parse(pid);
            if (!sModules)
            {
                std::cerr << "[Error] Failed to parse module map for PID " << pid << "\n";
                continue;
            }

            auto filtered = filter->filter(*sModules, config);
            if (!filtered)
            {
                std::cerr << "[Error] Filtering failed: " << static_cast<int>(filtered.error()) << "\n";
                continue;
            }

            modules = *filtered;
        }
        else if(nameProcess == "2")
        {
            std::cin >> pid;
            int val = 0;
            std::cout << "Enter value: \n";
            std::cin >> val;

            auto mem = std::make_shared<Memory>(pid);
            auto sModules = moduleParser.parse(pid);
            auto value = std::make_shared<Value>(val);
            if (!sModules)
            {
                std::cerr << "[Error] Failed to parse module map for PID " << pid << "\n";
                continue;
            }
            auto modules = std::make_shared<std::vector<MemoryRegion>>(*sModules);
            scanCfg.filter = filter;
            scanCfg.memory = mem;
            scanCfg.regions = modules;
            scanCfg.value = value;

            Scanner scan(scanCfg);

            auto result = scan.searchValue();

            if(!result)
            {
                std::cout << "Error search value \n";
                return 0;
            }

            auto& vecResult = scan.getResult();

            for(const auto& addr : vecResult)
            {
                std::cout << "result: " << std::hex << addr.address << " value: ";
for (auto b : addr.value) {
    // Печатаем каждый байт как число в HEX формате
    std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
}
std::cout << std::dec << std::endl;
            }
        }
        else
        {
            auto infoProc = finder.searhProcessInfoByFilter(nameProcess);
            if (!infoProc)
            {
                std::cerr << "[Error] No processes found matching: " << nameProcess << "\n";
                continue;
            }

            std::cout << "[Info] Found processes:\n";
            for (const auto& entry : *infoProc)
            {
                std::cout << "  Name: " << entry.name << " | PID: " << entry.pid << "\n";
            }
            continue;
        }

        // Вывод отфильтрованных модулей
        std::cout << "\n[Modules]\n";
        for (const auto& module : modules)
        {
            std::cout << "Start: 0x" << std::hex << module.start
                    << " End: 0x" << module.end << std::dec << "\n";
            std::cout << "Offset: " << module.offset
                    << " Perms: " << module.permissions << "\n";
            std::cout << "Path: " << module.pathname << "\n\n";
        }
    }

    std::cerr << "Exiting debug main.\n";
    return 0;
}