#include "ModuleFilter.hpp"

/**
 * @brief Фильтрует регионы памяти по правилам из ModuleFilterConfig
 *
 * Проходит по всем регионам и оставляет только те, которые удовлетворяют
 * включённым фильтрам: writable, executable, system, anonymous, drivers, TemporaryFile
 *
 * @param regions Вектор всех MemoryRegion, полученных от парсера
 * @param config Конфигурация фильтрации
 * @return std::expected<std::vector<MemoryRegion>, FilterError>
 * Отфильтрованный вектор регионов при успехе,
 * std::unexpected с FilterError при пустом входном или результативном векторе
 */
std::expected<std::vector<MemoryRegion>, FilterError> ModuleFilter::filter(const std::vector<MemoryRegion>& regions, const ModuleFilterConfig& config) const
{
    if(regions.empty())
        return std::unexpected{FilterError::InvalidIdentifier};

    std::vector<MemoryRegion> resultRegion{};

    for(const auto& region : regions)   
    {
        if(!matchWritable(region, config)) continue;
        if(!matchExecutable(region, config)) continue;
        if(!matchSystem(region, config)) continue;
        if(!matchAnonymous(region, config)) continue;
        if(!matchDrivers(region, config)) continue;
        if(!matchTemporaryFile(region, config)) continue;

        resultRegion.push_back(region);
    }
    if(resultRegion.empty())
        return std::unexpected{FilterError::FilteringError};

    return resultRegion;
}

/**
 * @brief Проверяет, удовлетворяет ли регион фильтру по праву на запись
 *
 * Если в конфиге onlyWritable == false, регион всегда считается прошедшим фильтр
 * Если onlyWritable == true, проверяет, есть ли флаг 'w' во втором символе permissions
 *
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Регион проходит фильтр
 * @return false Регион не проходит фильтр
 */
bool ModuleFilter::matchWritable(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    if(!config.onlyWritable) return true;

    return region.permissions.size() > 1 && region.permissions[1] == 'w';
}

/**
 * @brief Проверяет, удовлетворяет ли регион фильтру по праву на исполнение 
 * 
 * Если в confige onlyExecutable == false, регион всегда сщитается прошедшем фильтер
 * Если в onlyExecutable == true, проверяет, есть ли флаг 'x' во третьем символе permissions
 * 
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Регион проходит фильтр
 * @return false Регион не проходит фильтр
 */
bool ModuleFilter::matchExecutable(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    if(!config.onlyExecutable) return true;

    return region.permissions.size() > 2 && region.permissions[2] == 'x';
}

/**
 * @brief Проверяет является ли регион системной библиотекой
 * 
 * Если в confige excludeSystemLibs == false, регион всегда сщитается прошедшем фильтер
 * Если в excludeSystemLibs == true, проверяет, срабатывает ли паттерн для фильтра в pathname региона
 * паттерн("/usr/lib" && "/lib")
 * 
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Регион проходит фильтр
 * @return false Регион не проходит фильтр
 */
bool ModuleFilter::matchSystem(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    if(!config.excludeSystemLibs) return true;

    return !region.pathname.starts_with("/usr/lib") && !region.pathname.starts_with("/lib");
}

/**
 * @brief проверяет анонимный ли регион
 * 
 * Если config.includeAnonymous == true, регион всегда сщитается прошедшим фильтр
 * Если config.includeAnonymous == false, проверяет пустой ли pathname модуля
 * 
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Eсли регион НЕ анонимный
 * @return false Если регион анонимный
 */
bool ModuleFilter::matchAnonymous(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    return config.includeAnonymous ||  !region.pathname.empty();
}

/**
 * @brief Проверяет является ли регион драйвером
 * 
 * Если в confige includeDrivers == true, регион всегда сщитается прошедшем фильтер
 * Если в includeDrivers == false, проверяет, срабатывает ли паттерн для фильтра в pathname региона
 * паттерн("/dev/")
 * 
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Регион проходит фильтр
 * @return false Регион не проходит фильтр
 */
bool ModuleFilter::matchDrivers(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    return config.includeDrivers || !region.pathname.starts_with("/dev/");
}

/**
 * @brief Проверяет является ли регион временым файлом в памяти
 * 
 * Если в confige includeTemporaryFile == true, регион всегда сщитается прошедшем фильтер
 * Если в includeTemporaryFile == false, проверяет, срабатывает ли паттерн для фильтра в pathname региона
 * паттерн((delete) && "memfd")
 * 
 * @param region Проверяемый MemoryRegion
 * @param config Конфигурация фильтрации
 * @return true Регион проходит фильтр
 * @return false Регион не проходит фильтр
 */
bool ModuleFilter::matchTemporaryFile(const MemoryRegion& region, const ModuleFilterConfig& config) const noexcept
{
    return config.includeTemporaryFile || (!region.pathname.ends_with("(deleted)") &&
    region.pathname.find("memfd") == std::string::npos);
}