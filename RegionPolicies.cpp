#include "RegionPolicies.hpp"
/**
 * @brief Заполняет конфиг фильтрации регионов именно для сканирования
 * 
 * @return ModuleFilterConfig Заполненый кфг 
 */
ModuleFilterConfig RegionPolicies::forScan()
{
    ModuleFilterConfig config{};
    config.onlyWritable = true;
    config.onlyExecutable = false;
    config.excludeSystemLibs = true;
    config.includeAnonymous = true;
    config.includeDrivers = false;
    config.includeTemporaryFile = false;

    return config;
}