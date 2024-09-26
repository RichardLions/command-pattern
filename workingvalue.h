#pragma once

#include <cstdint>

class WorkingValue
{
public:
    using ValueType = int32_t;

    [[nodiscard]] ValueType GetValue() const
    {
        return m_Value;
    }

    void ModifyValue(const ValueType modification)
    {
        m_Value += modification;
    }
private:
    ValueType m_Value{0};
};
