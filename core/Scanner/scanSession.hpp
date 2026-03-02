#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <value.hpp>

struct ScanResult
{
    uintptr_t address;
    std::vector<uint8_t> value;
};

class ScanSessions
{
public:
    void clear() noexcept;
    size_t size() const noexcept;
    const std::vector<ScanResult>& getData() const noexcept;

    void filterByAddr(std::vector<uintptr_t>& newAddr);
    void add(uintptr_t addr, std::span<const uint8_t> value);

private:
    std::vector<ScanResult> result{};
};