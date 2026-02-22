#include "ProcessReader.hpp"
#include <filesystem>
#include <fstream>

/**
 * @brief читает имя процесса в /proc/pid/comm
 * 
 * @param pid индетификатор процесса 
 * @return std::expected<std::string, ProcessError> Имя процесса при удачном чтении
 * @retval ProcessError::InvalidIdentifier если pid процесса не положительный
 * @retval ProcessError::SourceUnavailable если недостаточно прав на открытие или при резком исчезновении процесса
 * @retval ProcessError::NotFound если имя процесса небыло найдено
 */
std::expected<std::string, ProcessError> ProcessReader::readProcessComm(pid_t pid) const 
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
std::expected<std::string, ProcessError> ProcessReader::readProcessCmdline(pid_t pid) const
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
 * @brief читает 1 байт из cmdline
 * 
 * @param pid индефитикатор процесса, откуда нужно прочитать 1 байт cmdline
 * @return true при успешном чтении 1 байта
 * @return false если pid процесса не положительный,
 * если недостаточно прав на открытие или при резком исчезновении процесса
 * если не удалось процитать 1 байт в cmdline
 */
/*bool ProcessReader::canReadCmdline(pid_t pid) const
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
}*/

std::expected<std::vector<std::string>, ProcessError> ProcessReader::readProcessMaps(pid_t pid) const 
{
    
}