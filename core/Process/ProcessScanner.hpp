#pragma once
#include "IProcess.hpp"

/**
 * @class ProcessScanner
 * @brief класс для чтения ФВС /proc,
 * класс предоставляет метод, для извлечения всех индетификаторов процессов в системе
 */

class ProcessScanner : public IProcessScanner
{
public:
    /**
     * @brief Парсит список Pid процессов в ВФС /proc
     * Проверяет наличие диреткории /proc(не пропалали случайно или недостаток прав) и итерируется по именам
     * 
     * @return std::expected<std::vector<pid_t>, ProcessError> вектор с Pidами всех процессов при успехе или ошибку
     */
    std::expected<std::vector<pid_t>, ProcessError> enumerateProcessPid() const;
private:
    std::expected<pid_t, ProcessError> parsePid(const std::string& name) const;
};