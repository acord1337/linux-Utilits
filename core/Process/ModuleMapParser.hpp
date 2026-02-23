#pragma once
#include "IProcess.hpp"
#include <cstdint>

struct MemoryRegion 
{
    uintptr_t start;
    uintptr_t end;
    std::string permissions;
    uintptr_t offset;
    std::string pathname;
};

class IModuleMapParser
{
public:
    virtual std::expected<std::vector<MemoryRegion>, ProcessError> parse(pid_t pid) const = 0;

    ~IModuleMapParser() = default;
};

class ModuleMapParser : public IModuleMapParser
{
public:
    explicit ModuleMapParser(const IProcessReader& reader);

    std::expected<std::vector<MemoryRegion>, ProcessError> parse(pid_t pid) const override;

private:
    std::expected<MemoryRegion, ProcessError> parseLine(const std::string& line) const;
    const IProcessReader& reader;
};
