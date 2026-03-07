#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "value.hpp"
#include "../Process/MemoryReader.hpp"
struct ScanResult
{
    uintptr_t address;
    std::vector<std::byte> value;
};

class ScanSessions
{
public:
    ScanSessions() noexcept = delete;
    ~ScanSessions() = default;

    ScanSessions(const ScanSessions&) = delete;
    ScanSessions& operator=(const ScanSessions&) = delete;

    ScanSessions(ScanSessions&&) noexcept = default;
    ScanSessions& operator=(ScanSessions&&) noexcept = default;

    explicit ScanSessions(Value val, Memory mem) noexcept;
    void clear() noexcept;
    [[nodiscard]] size_t size() const noexcept;
    [[nodiscard]]   const std::vector<ScanResult>& getData() const noexcept;

    void filterPrevious();
    void add(uintptr_t addr, std::span<const std::byte> value);

private:
    std::vector<ScanResult> result{};
    Value val;
    Memory mem;
};