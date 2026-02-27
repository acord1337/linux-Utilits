#pragma once
#include <vector>
#include <string>
#include <expected>
#include <cstdint>
#include "ModuleMapParser.hpp"

/**
 * @brief Возможные ошибки фильтрации модулей
 * 
 */
enum class FilterError
{
    FilteringError, // Входной Ошибка фильтрации, после прохода нет регионов
    InvalidIdentifier //Входной вектор регионов пуст
};

/**
 *@brief Конфигурация фильтрации модулей
 *
 * Определяет, какие регионы памяти должны быть включены при фильтрации
 * 
 * onlyWritable -- только rw-регионы
 * onlyExecutable -- только исполняемые регионы
 * excludeSystemLibs -- исключить системные библиотеки (/lib, /usr/lib)
 * includeAnonymous -- включать анонимные регионы ([heap], [stack])
 * includeDrivers -- включать регион, пренадлежищий к драйверу (/dev/*)
 * includeTemporaryFile -- включать временые файлы ((deleted))
 * processName -- имя процесса
 */
struct ModuleFilterConfig
{
    bool onlyWritable = true;
    bool onlyExecutable = false;
    bool excludeSystemLibs = true;
    bool includeAnonymous = true;
    bool includeDrivers = false;
    bool includeTemporaryFile = false;
    std::string processName{};
};

class IModuleFilter
{
public:
    virtual std::expected<std::vector<MemoryRegion>, FilterError>
    filter(const std::vector<MemoryRegion>& regions, const ModuleFilterConfig& config) const = 0;

    ~IModuleFilter() = default;
};

/**
 * @brief Фильтр модулей / регионов памяти процесса
 *
 * Предоставляет методы для фильтрации векторов MemoryRegion по разным условиям:
 * права доступа, системные библиотеки, анонимные регионы, драйверы и временные файлы.
 */
class ModuleFilter : public IModuleFilter
{
public:
    /**
     * @brief Фильтрует регионы памяти по правилам из ModuleFilterConfig
     * @param regions Вектор всех MemoryRegion
     * @param config Конфигурация фильтрации
     * @return std::expected<std::vector<MemoryRegion>, FilterError>
     * Вектор отфильтрованных регионов или ошибка FilterError
     */
    std::expected<std::vector<MemoryRegion>, FilterError>
    filter(const std::vector<MemoryRegion>& regions, const ModuleFilterConfig& config) const override;

private:
    /// @brief Проверка на право записи (rw)
    bool matchWritable(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;

    /// @brief Проверка на право исполнения (x)
    bool matchExecutable(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;

    /// @brief Проверка, что регион не является системной библиотекой
    bool matchSystem(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;

    /// @brief Проверка, что регион не является анонимным
    bool matchAnonymous(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;

    /// @brief Проверка, что регион не относится к драйверам (/dev/)
    bool matchDrivers(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;

    /// @brief Проверка, что регион не является временным файлом памяти (memfd, (deleted))
    bool matchTemporaryFile(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept;
};