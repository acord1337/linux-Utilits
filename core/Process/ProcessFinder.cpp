#include "ProcessFinder.hpp"
#include <ranges>
#include <algorithm>

ProcessFinder::ProcessFinder(
        const IProcessReader& reader,
        const IProcessScanner& scanner) : reader(reader), scanner(scanner) {}

/**
 * @brief Ищет все совпавшие процессы по вводу.
 * 
 * @param name Входная буква/строка.
 * @return std::expected<std::vector<ProcessInfo>, ProcessError> вектор структур с данными о процессе(имя, id).
 * @retval ProcessError::InvalidIdentifier если передоваемая строка или буква пуста.
 * @retval enumerateProcessPid().error если возникли какие либо проблемы при получении имен процессов в /proc
 * @retval tolowerString.error() при ошибки конвертации строки в нижний регистер
 */
std::expected<std::vector<ProcessInfo>, ProcessError>ProcessFinder::searhProcessInfoByFilter(std::string& name) const 
{
    if(name.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};

    auto tolowerName = tolowerString(name);

    if(!tolowerName)
        return std::unexpected{tolowerName.error()};

    std::vector<ProcessInfo> infoProcess{};

    auto pids = scanner.enumerateProcessPid();

    if (!pids)
        return std::unexpected{pids.error()};

    for(const auto& pid : *pids)
    {
        auto nameProcess = matchesProcessName(pid, *tolowerName);

        if(nameProcess && !nameProcess->empty())
        {
            infoProcess.push_back({*nameProcess, pid});
        }
    }

    return infoProcess;
}

/**
 * @brief Переделывает всю строку в нижний регистер
 * 
 * @param line строка или символ, который нужно привеси в нижний регистер
 * @return std::expected<std::string, ProcessError> Строку с нижним регистром при успехе
 * @retval ProcessError::InvalidIdentifier при неположительном pid процесса
 */
std::expected<std::string, ProcessError> ProcessFinder::tolowerString(std::string& line) const 
{
    if(line.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};
    std::string resultTolowerLine;
    resultTolowerLine.reserve(line.size());

    std::ranges::transform(line, std::back_inserter(resultTolowerLine), [](unsigned char s){ return static_cast<char>(std::tolower(s)); });

    return resultTolowerLine;
}

/**
 * @brief ищет все совпавшие процессы по фильтру в comm
 * 
 * @param pid индетификатор процесса
 * @param name фильтр, по которому надо искать
 * @return std::expected<std::string, ProcessError> строку при удачном нахождении процесса
 * @retval ProcessError::InvalidIdentifier если пид не положительный
 * @retval readProcessComm.error() || tolowerString.error() если возникла ошибка при чтении названия процесса или конвертации строки в нижний регистер
 * @retval ProcessError::NotFound еслии небыло совпадений
 */
std::expected<std::string, ProcessError> ProcessFinder::matchesProcessName(pid_t pid, const std::string& name) const
{
    if(pid <= 0 || name.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};

    auto nameComm = reader.readProcessComm(pid);
    if(!nameComm)
        return std::unexpected{nameComm.error()};

    if(nameComm->empty())
        return std::unexpected{ProcessError::NotFound};

    auto tolowerName = tolowerString(*nameComm);

    if(!tolowerName)
        return std::unexpected{tolowerName.error()};

    if(tolowerName->find(name) != std::string::npos)
        return *nameComm;

    return std::unexpected{ProcessError::NotFound};
}