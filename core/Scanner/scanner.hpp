#pragma once
#include "../Process/MemoryReader.hpp"
#include "../Process/ModuleFilter.hpp"
#include "scanSession.hpp"
#include "value.hpp"
#include <vector>
#include <span>
#include <cstddef>
#include <expected>


enum class ScanError
{
    InvalidIdentifier,
    InvalidRegion,
    ReadError,
};

enum class Alignment
{
    Byte = 1,
    Four = 4
};

class Scanner
{
public:
    explicit Scanner(size_t chunkSize = 16 * 1024 * 1024) noexcept;
    ~Scanner() = default;

    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;

    Scanner(Scanner&&) noexcept = default;
    Scanner& operator=(Scanner&&) noexcept = default;

    [[nodiscard]] std::expected<void, ScanError> scan
    (
        const std::vector<MemoryRegion>& regions,
        ScanSessions& sessions,
        const Value& value,
        Memory& memory
    ) const;

    [[nodiscard]] std::vector<ScanResult> scanAll
    (
        const std::vector<MemoryRegion>& allRegions,
        const Value& value,
        Memory& memory
    ) const noexcept;

    void setAlignment(Alignment a) noexcept;
private:
    mutable std::vector<std::byte> buffer{};

    size_t step = 4;

    template <typename T>
    void findMatches
    (
        const Value& value,
        uintptr_t base,
        size_t dataSize,
        T&& callBack
    ) const noexcept
    {
        size_t valSize = value.size();
        auto span = std::span(buffer).first(dataSize);

        for (size_t i = 0; i + valSize <= span.size(); i += step)
        {
            auto bytes = span.subspan(i, valSize);

            if(value.match(bytes, 0.1))
            {
                callBack(base + i, bytes);
            }
        }
    }
};