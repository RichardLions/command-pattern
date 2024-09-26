#pragma once

#include <memory>
#include <functional>

#include "workingvalue.h"

namespace ValueSemantics
{
    class ModifyValueCommand
    {
    public:
        ModifyValueCommand(std::shared_ptr<WorkingValue> value, const int32_t valueModification)
            : m_Value{value}
            , m_Modification{valueModification}
        {
        }

        void Execute()
        {
            m_Value->ModifyValue(m_Modification);
        }

        void Rollback()
        {
            m_Value->ModifyValue(-m_Modification);
        }
    private:
        std::shared_ptr<WorkingValue> m_Value{};
        WorkingValue::ValueType m_Modification{};
    };

    class LambdaCommand
    {
    public:
        using FunctionSignature = std::function<void()>;

        LambdaCommand(FunctionSignature&& execute, FunctionSignature&& rollback)
            : m_Execute{std::move(execute)}
            , m_Rollback{std::move(rollback)}
        {
        }

        void Execute()
        {
            m_Execute();
        }

        void Rollback()
        {
            m_Rollback();
        }
    private:
        FunctionSignature m_Execute{};
        FunctionSignature m_Rollback{};
    };
}
