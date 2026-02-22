#pragma once
#include <expected>
#include <vector>
#include "IProcess.hpp"

struct ProcessInfo 
{
    std::string name;
    pid_t pid = 0;
};

class ProcessFinder
{
public:
    explicit ProcessFinder(
        const IProcessReader& reader,
        const IProcessScanner& scanner);

    /**
     * @brief Находит все совпадающие процессы по указаному фильтру
     * 
     * @param name входной фильтер, по которому будет поиск
     * @retval std::vector<ProcessInfo> , ProcessError> вектор структур с данными о процессе при успехе
     * @retval ProcessError при ошибке
     */
    std::expected<std::vector<ProcessInfo>, ProcessError>searhProcessInfoByFilter(std::string& name) const;
private:
    const IProcessReader& reader;
    const IProcessScanner& scanner;

private:
/**
 * @brief ищет все совпавшие процессы по фильтру в /proc/pid/comm
 * 
 * @param pid индетификатор процесса
 * @param name фильтер, по которому будет проводиться поиск
 * @return std::expected<std::string, ProcessError> при успехе возвращает имя процесса
 * @retval если процесс не найден или возникла ошибка возвращается имя ошибки
 */
    std::expected<std::string, ProcessError> matchesProcessName(pid_t pid, const std::string& name) const;
    std::expected<std::string, ProcessError> tolowerString(std::string& line) const;
};