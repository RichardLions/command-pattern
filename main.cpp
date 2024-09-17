#include <functional>

#include "random.h"
#include "logger.h"

class WorkingValue final
{
public:
    using ValueType = int32_t;

    ValueType GetValue() const { return m_Value; }
    void AddToValue(const ValueType add) { m_Value += add; }
    void SubtractFromValue(const ValueType subtract) { m_Value -= subtract; }

private:
    ValueType m_Value{0};
};

class Command
{
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
};

// Work in progress
//class CommandQueue final
//{
//public:
//    void ExecuteNextCommand() {}
//
//    void UndoPreviousCommand() {}
//
//    bool HasPendingCommand() const
//    {
//        return m_Index > m_Commands.size(); // fix this
//    }
//
//    void PruneCommands()
//    {
//        const uint32_t count{m_Commands.size() - m_Index};
//        m_Commands.resize(count);
//    }
//
//    void AddCommand(std::shared_ptr<Command> command)
//    {
//        PruneCommands(); // If we have undone a command, there could be pending commands ahead that need removing before adding the new command
//        m_Commands.push_back(command);
//    }
//
//    std::vector<std::shared_ptr<Command>> m_Commands{};
//    uint32_t m_Index{0};
//};

class LambdaCommand final : public Command
{
public:
    using FunctionType = std::function<void()>;

    LambdaCommand(FunctionType&& function)
        : m_Function{std::move(function)}
    {
    }

    void Execute() override
    {
        Logger::LogMessage("Execute Lambda Command");
        m_Function();
    }
private:
    FunctionType m_Function{};
};

class AdditionCommand final : public Command
{
public:
    AdditionCommand(std::shared_ptr<WorkingValue> value)
        : m_Value{value}
        , m_Modification{Random::RandomInRange(1, 100)}
    {
    }

    void Execute() override
    {
        Logger::LogMessage("Execute AdditionCommand");
        m_Value->AddToValue(m_Modification);
    }
private:
    std::shared_ptr<WorkingValue> m_Value{};
    WorkingValue::ValueType m_Modification{};
};

class SubtractCommand final : public Command
{
public:
    SubtractCommand(std::shared_ptr<WorkingValue> value)
        : m_Value{value}
        , m_Modification{Random::RandomInRange(1, 100)}
    {
    }

    void Execute() override
    {
        Logger::LogMessage("Execute SubtractCommand");
        m_Value->SubtractFromValue(m_Modification);
    }
private:
    std::shared_ptr<WorkingValue> m_Value{};
    WorkingValue::ValueType m_Modification{};
};

static std::shared_ptr<Command> CreateRandomCommand(std::shared_ptr<WorkingValue> value)
{
    switch (Random::RandomInRange(0, 2))
    {
    case 0:
        return std::make_shared<LambdaCommand>([]{ Logger::LogMessage("Execute CommandLambda"); });
    case 1:
        return std::make_shared<AdditionCommand>(value);
    case 2:
    default:
        return std::make_shared<SubtractCommand>(value);
    }
}

int main()
{
    std::shared_ptr<WorkingValue> value{std::make_shared<WorkingValue>()};

    // show undo working
    constexpr uint32_t limit{100};
    for(uint32_t i{0}; limit > i; ++i)
    {
        std::shared_ptr<Command> command{CreateRandomCommand(value)};
        command->Execute();
    }
}
