#include "scanSession.hpp"
#include <unordered_set>
#include <ranges>
#include <algorithm>
#include <span>

ScanSessions::ScanSessions(Value val, Memory mem) noexcept : mem(std::move(mem)) {}

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
    return result;
}


void ScanSessions::add(uintptr_t addr, std::span<const std::byte> value)
{
    if(addr == 0 || value.empty()) return;

    result.push_back({addr, std::vector<std::byte>(value.begin(), value.end())});
}

void ScanSessions::filterPrevious(const Value& val)
{
    if(result.empty())
        return;

    std::erase_if(result, [&](ScanResult& addr)
    {
        std::vector<std::byte> buffer(val.size());

        auto readByte = mem.readBlock(addr.address, val.size(), buffer.data());

        if(!readByte) return true;

        if(!val.match(std::span(buffer), 0.1))
            return true;

        addr.value = std::move(buffer);
        return false;
    });
}