#pragma once
#include "../Process/MemoryReader.hpp"
#include "../Process/ModuleFilter.hpp"
#include "TheardScann.hpp"
#include "value.hpp"
#include <memory>

enum class ScanError
{
    InvalidIdentifier,
    InvalidRegion,
    ReadError,
};

struct ScanerContext
{
    std::shared_ptr<std::vector<MemoryRegion>> regions;
    std::shared_ptr<ModuleFilter> filter;
    std::shared_ptr<Memory> memory;
    std::shared_ptr<ThreadScan> threadScan;
    std::shared_ptr<Value> value;
};

struct ScanResult
{
    uintptr_t address;
    std::vector<uint8_t> value;
};

class Scanner
{
public:
    explicit Scanner(ScanerContext sessions);
std::expected<void, ScanError> searchValue();
    std::expected<std::vector<MemoryRegion>, ScanError> getRegions() const;
    std::expected<void, ScanError> scanBuffer(const uint8_t* buffer, size_t size, uintptr_t baseAddr);

    std::vector<ScanResult>& getResult();

private:
    std::vector<ScanResult> result;
    ScanerContext tool;
};