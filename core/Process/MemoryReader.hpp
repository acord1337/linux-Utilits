#pragma once
#include <sys/uio.h>
#include <type_traits>
#include <expected>
#include <cstdint>

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v <T>;

enum class MemoryError
{
    InvalidIdentifier,
    ReadError
};

class Memory
{
private:
    pid_t pid {0};
public:
explicit Memory(pid_t pid);

template <TriviallyCopyable T>
std::expected<T,MemoryError> readProcess(const uintptr_t addr) const
{
    T buffer{};

    if (pid <= 0)
        return std::unexpected{MemoryError::InvalidIdentifier};

    iovec local
    {
        .iov_base = &buffer,
        .iov_len = sizeof(T)
    };

    iovec remote
    {
        .iov_base = reinterpret_cast<void*>(addr),
        .iov_len = sizeof(T)
    };

    ssize_t read = process_vm_readv(pid, &local, 1, &remote, 1, 0);

    if (read != sizeof(T))
        return std::unexpected{MemoryError::ReadError};

    return buffer;
}

template <TriviallyCopyable T>
std::expected<void, MemoryError> writeProcess(const uintptr_t addr, const T& value) const
{
    if(pid <= 0)
        return std::unexpected{MemoryError::InvalidIdentifier};

    struct iovec local_iov
    {
        .iov_base = const_cast <void*> (static_cast<const void*>(&value)),
        .iov_len = sizeof(T)
    };
    struct iovec remote_iov
    {
        .iov_base = reinterpret_cast <void*> (addr),
        .iov_len = sizeof(T)
    };

    if(process_vm_writev(pid, &local_iov, 1, &remote_iov, 1, 0) != sizeof(value))
        return std::unexpected{MemoryError::ReadError};
}
};