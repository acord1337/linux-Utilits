#include "value.hpp"
#include <cstring>

/**
 * @brief Возвращает размер  текущего хранимого типа в байтах
 * 
 * Метод автоматически учитывает тип данных и возвращает размер
 * 
 * @return size_t Размер типа в байтах
 */
size_t Value::size() const noexcept
{
    return std::visit([](auto&& arg) noexcept { return sizeof(arg); }, value);
}

/**
 * @brief Сравнивает хранимое значение с байтами по указаному адрессу
 * 
 * Метод автоматически учитывает тип данных. Для целых чисел выполняется 
 * точное сравнение, для чисел с плавающей точкой -- сравнение с учетом погрешности
 * 
 * @param memory Укаатель на участок памяти для проверки
 * @param epsilon Допустимая погрешность для float/double 
 * @return true Значения совпадают
 * @return false Значения не совпадают
 */
bool Value::match(const uint8_t* memory, double epsilon) const
{
    return std::visit([&](auto&& arg) -> bool 
    {
        using T = std::decay_t<decltype(arg)>;

        T memValue;
        std::memcpy(&memValue, memory, sizeof(T));

        if constexpr(std::is_floating_point_v<T>)
            return std::abs(memValue - arg) < static_cast<T>(epsilon);

        else
            return memValue == arg;

    }, value);
}