#pragma once
#include <sys/uio.h>
#include <type_traits>
#include <expected>
#include <cstdint>
#include <vector>

/**
 * @brief Допускает только типы, которые можно безопасно копировать побайтово
 * 
 * Запрещает типы с умным управлением памятью (std::string, std::vector и тп)
 * 
 * @tparam T Проверяемый тип данных.
 */
template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

/**
 * @brief Возможные ошибки сканирования памяти
 * 
 */
enum class MemoryError
{
    InvalidIdentifier, // pid не инцелезированый 
    ReadError // произошла ошибка при чтение памяти
};


/**
 * @brief Читает и записывает в память процесса по указаному адресу
 */
class Memory
{
private:
    pid_t pid {0};
public:
explicit Memory(pid_t pid) : pid(pid) {};

/**
 * @brief Чтение памяти процесса по указаному адресу
 * 
 * @tparam T тип данных для чтения
 * @param addr адрес, который надо прочитать
 * @return std::expected<T,MemoryError> 
 * возвращает размер буффера или ошибка ReadError при неудачном чтении
 */
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

/**
 * @brief Записывает в память процесса указаное значение по указаному адрессу
 * 
 * @tparam T тип данных для записи
 * @param addr адрес куда надо записать значение
 * @param value значение, которое будет записано
 * @return std::expected<void, MemoryError> 
 * При удачной записи ничего не возвращает, при ошибке записи ReadError
 */
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

/**
 * @brief Читает указаный кусок памяти процесса
 * 
 * @param addr от куда начать чтение
 * @param size сколько байтов прочитать
 * @return std::expected<std::vector<uint8_t>,MemoryError> 
 * При удачном чтении возвращает вектор с данными, при ошибке чтения -- ReadError
 */
std::expected<std::vector<uint8_t>,MemoryError> readBlock(const uintptr_t addr, size_t size) const
{
    if (pid <= 0 || size == 0)
        return std::unexpected{MemoryError::InvalidIdentifier};

    std::vector<uint8_t> buffer(size);

    iovec local
    {
        .iov_base = buffer.data(),
        .iov_len = size
    };

    iovec remote
    {
        .iov_base = reinterpret_cast<void*>(addr),
        .iov_len = size
    };

    ssize_t read = process_vm_readv(pid, &local, 1, &remote, 1, 0);

    if (read <= 0)
        return std::unexpected{MemoryError::ReadError};

    buffer.resize(read);
    return buffer;
}
};