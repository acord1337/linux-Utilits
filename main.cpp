#include <iostream>
#include <iomanip>
#include <memory>

#include "core/Process/ProcessFinder.hpp"
#include "core/Process/ProcessReader.hpp"
#include "core/Process/ProcessScanner.hpp"
#include "core/Process/ModuleMapParser.hpp"
#include "core/Process/MemoryReader.hpp"
#include "core/Process/ModuleFilter.hpp"

#include "core/Scanner/scanner.hpp"
#include "core/Scanner/value.hpp"
#include "core/Scanner/scanSession.hpp"

int main()
{
    ProcessScanner procScanner;
    ProcessReader reader;
    ProcessFinder finder(reader, procScanner);
    ModuleMapParser moduleParser(reader);
    ModuleFilter filter;

    Scanner scanner(16 * 1024 * 1024);

    pid_t pid{};
    std::string input;

    ModuleFilterConfig config;
    config.onlyWritable = true;
    config.onlyExecutable = false;
    config.excludeSystemLibs = true;
    config.includeAnonymous = true;

    std::vector<MemoryRegion> regions;

    while (true)
    {
        std::cout << "\nEnter process name | PID | 'q': ";
        std::cin >> input;

        if (input == "q")
            break;

        // -------------------------
        // PROCESS BY PID
        // -------------------------

        if (std::isdigit(input[0]))
        {
            pid = std::stoi(input);

            auto parsed = moduleParser.parse(pid);
            if (!parsed)
            {
                std::cerr << "parse modules failed\n";
                continue;
            }

            auto filtered = filter.filter(*parsed, config);
            if (!filtered)
            {
                std::cerr << "filter modules failed\n";
                continue;
            }

            regions = *filtered;

            std::cout << "[regions] " << regions.size() << "\n";
        }

        // -------------------------
        // PROCESS BY NAME
        // -------------------------

        else
        {
            auto proc = finder.searhProcessInfoByFilter(input);

            if (!proc || proc->empty())
            {
                std::cout << "process not found\n";
                continue;
            }

            for (auto& p : *proc)
                std::cout << p.name << " pid=" << p.pid << "\n";

            continue;
        }

        // -------------------------
        // FIRST SCAN
        // -------------------------

        std::cout << "Enter value: ";

        int valueInput{};
        std::cin >> valueInput;

        Memory mem(pid);
        Value value(valueInput);
        ScanSessions session(value, mem);

        session.clear();

        scanner.setAlignment(Alignment::Four);

        auto result = scanner.scan(
            regions,
            session,
            value,
            mem
        );

        if(!result)
        {
            std::cerr << "Ошибка scanner.scan \n";
            return 0;
        }

        std::cout << "found: " << session.size() << "\n";

        for (auto& r : session.getData())
        {
            std::cout
                << "addr 0x"
                << std::hex
                << r.address
                << std::dec
                << " value: ";

            for (auto b : r.value)
            {
                std::cout
                    << std::setw(2)
                    << std::setfill('0')
                    << std::hex
                    << (int)b
                    << " ";
            }

            std::cout << std::dec << "\n";
        }

        // -------------------------
        // NEXT SCAN LOOP
        // -------------------------

        while (true)
        {
            std::cout << "\n[n] next scan | [r] restart | [q] quit : ";

            std::cin >> input;

            if (input == "q")
                return 0;

            if (input == "r")
                break;

            if (input == "n")
            {
                int newValue = 0;
                std::cout << "ВВеди число: " << std::endl;
                std::cin >> newValue;
                value.setValue(newValue);
                session.filterPrevious();
                for (auto& r : session.getData())
        {
            std::cout
                << "addr 0x"
                << std::hex
                << r.address
                << std::dec
                << " value: ";

            for (auto b : r.value)
            {
                std::cout
                    << std::setw(2)
                    << std::setfill('0')
                    << std::hex
                    << (int)b
                    << " ";
            }

            std::cout << std::dec << "\n";
        }

                std::cout << "remaining: " << session.size() << "\n";
            }
        }
    }

    return 0;
}