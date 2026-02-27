#pragma once
#include <variant>
#include <cstdint>
#include <cmath>

/**
 * @brief Ограничивает допустимые типы данных для сканирования 
 * 
 * Концепт, разрешающий только целочисленные типы и типы с плавающей точкой
 * 
 * @tparam T передоваемый тип данных
 */
template <typename T>
concept ValidValueType =(std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>,
                        std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t>,
                        std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>,
                        std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>,
                        std::is_same_v<T, float> || std::is_same_v< T, double>);

/**
 * @brief 
 * 
 */
class Value 
{
public:
    /**
    * @brief набор поддеживаемых типов данных
    * 
    */
    using ValueVariant = std::variant<
        int8_t, uint8_t, 
        int16_t, uint16_t, 
        int32_t, uint32_t, 
        int64_t, uint64_t, 
        float, double
    >;

    template<ValidValueType T>
    explicit Value(T val) : value(val) {}

    size_t size() const noexcept;

    /**
     * @brief Сравнивает хранимое значение с байтами по указаному адрессу
     * 
     * Метод сам учитывает тип данных. Для целых чисел выполняется точное сравнение,
     * Для чисел с плавающей точкой -- сравнение с учетом погрешности
     * 
     * @param memory Указатель на начало адресса участка памяти для проверки
     * @param epsilon Допустимая погрешность для float and double
     * @return true Значения совпадают
     * @return false Значения не совпадают
     */
    bool match(const uint8_t* memory, double epsilon = 1e-6) const;

private:
    ValueVariant value;
};