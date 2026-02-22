#pragma once
#include "IProcess.hpp"

/**
 * @class ProcessReader
 * @brief Класс для чтения данных процессов по их индефитикатору
 * класс предстовляет методы для: извлечения имени процесса(comm,cmdline), получение модулей процесса по его пид
 */

class ProcessReader : public IProcessReader
{
public:
    std::expected<std::string, ProcessError> readProcessComm(pid_t pid) const override;
    std::expected<std::string, ProcessError> readProcessCmdline(pid_t pid) const override;
    std::expected<std::vector<std::string>, ProcessError> readProcessMaps(pid_t pid) const override;
};