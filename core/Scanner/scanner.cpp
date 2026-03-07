#include "scanner.hpp"
#include <span>

Scanner::Scanner(size_t chunkSize) noexcept : buffer(chunkSize) {}

std::expected<void, ScanError> Scanner::scan
(
    const std::vector<MemoryRegion>& regions,
    ScanSessions& sessions,
    const Value& value,
    Memory& memory
) const
{
    for (const auto& reg : regions)
    {
        size_t offset = 0;
        while (offset < reg.size())
        {
            size_t size = std::min(buffer.size(), reg.size() - offset);
            auto readBytes = memory.readBlock(reg.start + offset, size, buffer.data());

            if(!readBytes) return std::unexpected{ScanError::ReadError};
            if(*readBytes == 0) break;

            findMatches
            (
                value, reg.start + offset, *readBytes, [&](uintptr_t addr, auto bytes)
            {
            sessions.add(addr, bytes);
            });

            offset += *readBytes;
        }
    }
    return {};
}

void Scanner::setAlignment(Alignment a) noexcept
{
    step = static_cast<size_t>(a);
}