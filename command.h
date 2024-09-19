#pragma once

#include <functional>

class Command
{
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
};

class LambdaCommand final : public Command
{
public:
    using FunctionSignature = std::function<void()>;

    LambdaCommand(FunctionSignature&& function)
        : m_Function{std::move(function)}
    {
    }

    void Execute() override
    {
        m_Function();
    }
private:
    FunctionSignature m_Function{};
};
