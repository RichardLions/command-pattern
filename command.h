#pragma once

#include <functional>

class Command
{
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Rollback() = 0;
};

class LambdaCommand final : public Command
{
public:
    using FunctionSignature = std::function<void()>;

    LambdaCommand(FunctionSignature&& execute, FunctionSignature&& rollback)
        : m_Execute{std::move(execute)}
        , m_Rollback{std::move(rollback)}
    {
    }

    void Execute() override
    {
        m_Execute();
    }

    void Rollback() override
    {
        m_Rollback();
    }
private:
    FunctionSignature m_Execute{};
    FunctionSignature m_Rollback{};
};
