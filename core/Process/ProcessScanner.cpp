#include "ProcessScanner.hpp"
#include <filesystem>
#include <errno.h>
#include <charconv>

/**
 * @brief перечисляет все запущеные процессы в /proc
 * 
 * @return std::expected<std::vector<pid_t>, ProcessError> Вектор в pid процессов
 * @retval ProcessError::SourceUnavailable если директория /proc отсутствует или недоступна
 * @retval ProcessError::ReadError если возникла ошибка при чтении содержимого.
 */
std::expected<std::vector<pid_t>, ProcessError> ProcessScanner::enumerateProcessPid() const
{
    std::vector<pid_t> pids{};
    std::filesystem::path path = std::filesystem::path{"/proc"};
    std::error_code ec;

    if(!std::filesystem::exists(path, ec) || !std::filesystem::is_directory(path, ec))
        return std::unexpected{ProcessError::SourceUnavailable};

    auto it = std::filesystem::directory_iterator(path, ec);

    if(ec)
        return std::unexpected{ProcessError::SourceUnavailable};

    for(const auto& entry : it)
    {
        if(!entry.exists() || !entry.is_directory())
            continue;

        std::string name = entry.path().filename().string();

        auto pid = parsePid(name);

        if(!pid)
            continue;

        pids.push_back(*pid);
    }

    if(ec)
        return std::unexpected{ProcessError::ReadError};

    return pids;
}

/**
 * @brief преобразует строку пид процесса в pid_t, отсеивая не целочисленые имена директории
 * @param name Входная строка с именем процесса
 * @return std::unexpected<pid_t, ProcessError> Pid процесса в случае успеха
 * @retval ProcessError::InvalidIndentifier если имя директории пустое или директория содержит не только числа,
 * pid выходит за границы диапозона
 */
std::expected<pid_t, ProcessError>ProcessScanner::parsePid(const std::string& name) const
{
    if(name.empty())
            return std::unexpected{ProcessError::InvalidIdentifier};

    pid_t resultPid;
    auto [pointer, ec] = std::from_chars(name.data(), name.data() + name.size(), resultPid);

    if(ec != std::errc{} || pointer != name.data() + name.size())
        return std::unexpected{ProcessError::InvalidIdentifier};

    return resultPid;
}