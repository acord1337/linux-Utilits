#include "ModuleMapParser.hpp"
#include <sstream>
#include <charconv>

ModuleMapParser::ModuleMapParser(const IProcessReader& reader) : reader(reader) {}

std::expected<std::vector<MemoryRegion>, ProcessError> ModuleMapParser::parse(pid_t pid) const
{
    if(pid <= 0)
        return std::unexpected{ProcessError::InvalidIdentifier};

    auto vecModulesMap = reader.readProcessMaps(pid);

    if(!vecModulesMap)
        return std::unexpected{vecModulesMap.error()};

    if(vecModulesMap->empty())
        return std::unexpected{ProcessError::SourceUnavailable};

    std::vector<MemoryRegion> region{};

    for(const auto& module : *vecModulesMap)
    {
        auto sModuleMap = parseLine(module);

        if(!sModuleMap) return std::unexpected{sModuleMap.error()};

        region.push_back(std::move(*sModuleMap));
    }

    if(region.empty())
        return std::unexpected{ProcessError::ReadError};

    return region;
}

std::expected<MemoryRegion, ProcessError> ModuleMapParser::parseLine(const std::string& line) const
{
    if(line.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};

    //7f6f8a200000-7f6f8a225000 r--p 00000000 08:01 131075  /usr/lib/Dalbaeb

    std::stringstream cinString(line);
    std::string addres, perms, offsets, dev, inode, path;

    if(!(cinString >> addres >> perms >> offsets >> dev >> inode))
        return std::unexpected{ProcessError::ReadError};

    std::getline(cinString >> std::ws, path);

    if(auto dashPos = addres.find("-");
    dashPos != std::string::npos || dashPos > 0)
    {
    uintptr_t startAddr = 0, endAddr = 0, resOffsets = 0;

    if(auto [ptr, ec] = std::from_chars(addres.data(), addres.data() + dashPos, startAddr, 16);
    ec != std::errc{})  return std::unexpected{ProcessError::ReadError};

    if(auto [ptrEnd, ecEnd] = std::from_chars(addres.data() + dashPos + 1, addres.data() + addres.size(), endAddr, 16);
    ecEnd != std::errc{}) return std::unexpected{ProcessError::ReadError};

    if(auto [ptrOffsets, ecOffsets] = std::from_chars(offsets.data(), offsets.data() + offsets.size(), resOffsets, 16);
    ecOffsets != std::errc{}) return std::unexpected{ProcessError::ReadError};

    return MemoryRegion{ startAddr, endAddr, perms, resOffsets, path};
    }

    return std::unexpected{ProcessError::ReadError};
}