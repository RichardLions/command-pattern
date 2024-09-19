#pragma once

#include <catch2/catch_test_macros.hpp>

#include "command.h"
#include "commandqueue.h"
#include "logger.h"
#include "random.h"

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

TEST_CASE("Command Queue", "")
{
    std::shared_ptr<WorkingValue> value{std::make_shared<WorkingValue>()};
    CommandQueue queue{};
    REQUIRE_FALSE(queue.HasPendingCommand());
    REQUIRE(queue.GetCommandIndex() == 0);
    REQUIRE(queue.GetCommandQueueSize() == 0);

    SECTION("Add Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }

    SECTION("Execute Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());

        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 4);

        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 4);
        REQUIRE(queue.GetCommandQueueSize() == 4);
    }

    SECTION("Clear Command Queue")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 3);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ClearQueue();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 0);
    }

    SECTION("Prune Command Queue")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteNextCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 2);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.ExecuteNextCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 2);
    }
}
