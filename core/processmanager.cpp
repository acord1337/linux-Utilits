#include "processmanager.h"
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <fstream>
#include <charconv>
#include <system_error>

ProcessFinder::ProcessFinder(ProcessSource& source) : source(source) {}

/**
 * @brief перечисляет все запущеные процессы в /proc
 * 
 * @return std::expected<std::vector<pid_t>, ProcessError> Вектор в pid процессов
 * @retval ProcessError::SourceUnavailable если директория /proc отсутствует или недоступна
 * @retval ProcessError::ReadError если возникла ошибка при чтении содержимого.
 */
std::expected<std::vector<pid_t>, ProcessError> ProcessSource::enumerateProcessPid() const
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

        if(!isValidDirProcess(*pid))
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
std::expected<pid_t, ProcessError>ProcessSource::parsePid(const std::string& name) const
{
    if(name.empty())
            return std::unexpected{ProcessError::InvalidIdentifier};

    pid_t resultPid;
    auto [pointer, ec] = std::from_chars(name.data(), name.data() + name.size(), resultPid);

    if(ec != std::errc{} || pointer != name.data() + name.size())
        return std::unexpected{ProcessError::InvalidIdentifier};

    return resultPid;
}

/**
 * @brief проверяет является ли директория валидным процессом /proc
 * 
 * @param pid числовой интификатор процесса 
 * @return true если pid положителен  и удалось прочитать 1 байт в cmdline
 * @return false если pid не положительный, либо это служебный поток/недоступен
 */
bool ProcessSource::isValidDirProcess(pid_t pid) const 
{
    if(pid <= 0)
        return false;

    return canReadCmdline(pid);
}

/**
 * @brief читает 1 байт из cmdline
 * 
 * @param pid индефитикатор процесса, откуда нужно прочитать 1 байт cmdline
 * @return true при успешном чтении 1 байта
 * @return false если pid процесса не положительный,
 * если недостаточно прав на открытие или при резком исчезновении процесса
 * если не удалось процитать 1 байт в cmdline
 */
bool ProcessSource::canReadCmdline(pid_t pid) const
{
    if(pid <= 0)
        return false;

    std::filesystem::path cmdPath = std::filesystem::path("/proc") / std::to_string(pid) / "cmdline";

    if(!std::filesystem::exists(cmdPath) || !std::filesystem::is_regular_file(cmdPath))
        return false;

    std::ifstream file(cmdPath, std::ios::binary);

    if(!file.is_open())
        return false;

    char firstByteName;

    if(!(file >> firstByteName))
        return false;

    return true;
}

std::expected<std::vector<std::string>, ProcessError> ProcessSource::readProcessMaps(pid_t pid) const 
{
}

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

    auto tolowerName = source.tolowerString(name);

    if(!tolowerName)
        return std::unexpected{tolowerName.error()};

    std::vector<ProcessInfo> infoProcess{};

    auto pids = source.enumerateProcessPid();

    if (!pids)
        return std::unexpected{pids.error()};

    for(const auto& pid : *pids)
    {
        auto nameProcess = source.matchesProcessName(pid, *tolowerName);

        if(nameProcess && !nameProcess->empty())
        {
            infoProcess.push_back({*nameProcess, pid});
        }
    }

    return infoProcess;
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
std::expected<std::string, ProcessError> ProcessSource::matchesProcessName(pid_t pid, const std::string& name) const
{
    if(pid <= 0 || name.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};

    auto nameComm = readProcessComm(pid);
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

/**
 * @brief читает имя процесса в /proc/pid/comm
 * 
 * @param pid индетификатор процесса 
 * @return std::expected<std::string, ProcessError> Имя процесса при удачном чтении
 * @retval ProcessError::InvalidIdentifier если pid процесса не положительный
 * @retval ProcessError::SourceUnavailable если недостаточно прав на открытие или при резком исчезновении процесса
 * @retval ProcessError::NotFound если имя процесса небыло найдено
 */
std::expected<std::string, ProcessError> ProcessSource::readProcessComm(pid_t pid) const 
{
    if(pid <= 0)
        return std::unexpected{ProcessError::InvalidIdentifier};

    std::filesystem::path commPath = std::filesystem::path("/proc") / std::to_string(pid) / "comm";

    if(!std::filesystem::exists(commPath) || !std::filesystem::is_regular_file(commPath))
        return std::unexpected{ProcessError::SourceUnavailable};

    std::ifstream file(commPath);

    if(!file.is_open())
        return std::unexpected{ProcessError::SourceUnavailable};

    std::string nameProc;

    if(std::getline(file, nameProc))
    {
        if(!nameProc.empty())
            return nameProc;
    }

    return std::unexpected{ProcessError::NotFound};
}

/**
 * @brief читает имя процесса из cmdline
 * 
 * @param pid индетификатор процесса
 * @return std::expected<std::string, ProcessError> строку с именем процесса при успеке
 * @retval InvalidIdentifier если пид не положительный
 * @retval SourceUnavailable если недостаточно прав на открытие или при резком исчезновении процесса
 * @retval ProcessError::NotFound если имя процесса небыло найдено
 */
std::expected<std::string, ProcessError> ProcessSource::readProcessCmdline(pid_t pid) const
{
    if(pid <= 0)
        return std::unexpected{ProcessError::InvalidIdentifier};

    std::filesystem::path path = std::filesystem::path("/proc") / std::to_string(pid) / "cmdline";
    if(!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        return std::unexpected{ProcessError::SourceUnavailable};

    std::ifstream file(path);

    if(!file.is_open())
        return std::unexpected{ProcessError::SourceUnavailable};

    std::string fullName;
    std::string bufferName;

    while(std::getline(file, bufferName, '\0'))
    {
        fullName += bufferName + " ";
    }
    if(fullName.empty())
        return std::unexpected{ProcessError::NotFound};

    return fullName;
}

/**
 * @brief фильтрует имена процесса в cmdline, отбрасывая служебные скрипты
 * 
 * @param line строка, которую будем фильтровать
 * @return true при успешно фильтрации
 * @return false если строка пустая или не прошла фильтер
 */
bool ProcessSource::filterProcessCmdLine(const std::string& line) const
{
    if(line.empty()) return false;
    if(line.find("/bin/sh") != std::string::npos) return false;
    if(line.find("python") != std::string::npos) return false;
    return true;
}

/**
 * @brief Переделывает всю строку в нижний регистер
 * 
 * @param line строка или символ, который нужно привеси в нижний регистер
 * @return std::expected<std::string, ProcessError> Строку с нижним регистром при успехе
 * @retval ProcessError::InvalidIdentifier при неположительном pid процесса
 */
std::expected<std::string, ProcessError> ProcessSource::tolowerString(std::string& line) const 
{
    if(line.empty())
        return std::unexpected{ProcessError::InvalidIdentifier};
    std::string resultTolowerLine;
    resultTolowerLine.reserve(line.size());

    std::ranges::transform(line, std::back_inserter(resultTolowerLine), [](unsigned char s){ return static_cast<char>(std::tolower(s)); });

    return resultTolowerLine;
}