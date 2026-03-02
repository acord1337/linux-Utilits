#include "scanSession.hpp"
#include <unordered_set>
#include <ranges>
#include <algorithm>

void ScanSessions::clear() noexcept
{
    result.clear();
}

size_t ScanSessions::size() const noexcept
{
    return result.size();
}

const std::vector<ScanResult>& ScanSessions::getData() const noexcept
{
    if(result.empty())
        return;

    return result;
}

void ScanSessions::filterByAddr(std::vector<uintptr_t>& newAddr)
{
    if(result.empty() || newAddr.empty())
        return;

    std::ranges::sort(newAddr);

    std::erase_if(result, [&](const ScanResult& res)
    { return !std::ranges::binary_search(newAddr, res.address); });
}

void ScanSessions::add(uintptr_t addr, std::span<const uint8_t> value)
{
    if(addr == 0 || value.empty()) return;

    result.push_back({addr, std::vector<uint8_t>(value.begin(), value.end())});
}