#pragma once
#include "core/Process/ModuleFilter.hpp"

/**
 * @brief выбор политики для настройки конфига фильтрации регионов
 * 
 */
namespace RegionPolicies
{
    ModuleFilterConfig forScan() noexcept; /// предназначен для сканнера, возвращает заполненый cfg
}
