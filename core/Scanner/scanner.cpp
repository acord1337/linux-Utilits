#include "scanner.hpp"

Scanner::Scanner(ScanerContext sessions) : tool(std::move(sessions)) {}

std::expected<void, ScanError> Scanner::searchValue()
{
    constexpr size_t ChunkSize = 1024 * 1024;
    std::vector<uint8_t> buffer(ChunkSize);

    auto regions = getRegions();
    if (!regions)
        return std::unexpected{regions.error()};

    for (const auto& region : *regions)
    {
        if (region.start >= region.end)
            continue; 

        for (uintptr_t current = region.start; current < region.end;)
        {
            const size_t remaining = region.end - current;
            const size_t toRead = std::min(ChunkSize, remaining);

            if (!tool.memory->readBlock(current, toRead, buffer.data()))
                return std::unexpected{ScanError::ReadError};

            auto res = scanBuffer(buffer.data(), toRead, current);

            if(!res)
                return std::unexpected{res.error()};

            current += toRead;
        }
    }

    return {};
}

std::expected<void, ScanError> Scanner::scanBuffer(const uint8_t* buffer, size_t size, uintptr_t baseAddr)
{
    if(buffer == nullptr)
        return std::unexpected{ScanError::InvalidIdentifier};

    const size_t step = tool.value->size();

    for (size_t i = 0; i + step <= size; ++i)
    {
        if(tool.value->match(buffer + i, 0.1))
        {
            ScanResult res;
            res.address = baseAddr + i;
            res.value.assign(buffer + i, buffer + i + step);
            result.push_back(std::move(res));
        }
    }
    return {};
}

std::expected <std::vector<MemoryRegion>, ScanError> Scanner::getRegions() const
{
    if(tool.regions->empty())
        return std::unexpected{ScanError::InvalidIdentifier};

    ModuleFilterConfig config;
    config.onlyWritable = true;
    config.onlyExecutable = false;
    config.excludeSystemLibs = true;
    config.includeAnonymous = true;
    config.includeDrivers = false;
    config.includeTemporaryFile = false;

    auto regions = tool.filter->filter(*tool.regions, config);

    if(!regions)
        return std::unexpected{ScanError::InvalidRegion};

    return *regions;
}

std::vector<ScanResult>& Scanner::getResult()
{
    return result;
}