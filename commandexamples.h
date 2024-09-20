#include <catch2/catch_test_macros.hpp>

#include "command.h"
#include "commandqueue.h"
#include "logger.h"
#include "random.h"

namespace
{
    class WorkingValue final
    {
    public:
        using ValueType = int32_t;

        [[nodiscard]] ValueType GetValue() const { return m_Value; }
        void Add(const ValueType add)
        {
            const ValueType before{m_Value};
            m_Value += add;
            Logger::LogMessage(std::format("WorkingValue: Add({} + {} == {})", before, add, m_Value));
        }

        void Subtract(const ValueType subtract)
        {
            const ValueType before{m_Value};
            m_Value -= subtract;
            Logger::LogMessage(std::format("WorkingValue: Subtract({} - {} == {})", before, subtract, m_Value));
        }
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
            m_Value->Add(m_Modification);
        }

        void Rollback() override
        {
            Logger::LogMessage("Rollback AdditionCommand");
            m_Value->Subtract(m_Modification);
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
            m_Value->Subtract(m_Modification);
        }

        void Rollback() override
        {
            Logger::LogMessage("Rollback SubtractCommand");
            m_Value->Add(m_Modification);
        }
    private:
        std::shared_ptr<WorkingValue> m_Value{};
        WorkingValue::ValueType m_Modification{};
    };

    [[nodiscard]] static std::shared_ptr<Command> CreateRandomCommand(std::shared_ptr<WorkingValue> value)
    {
        switch (Random::RandomInRange(0, 2))
        {
        case 0:
            {
                const WorkingValue::ValueType modification{Random::RandomInRange(1, 100)};
                return std::make_shared<LambdaCommand>(
                    [value, modification]
                    {
                        Logger::LogMessage("Execute CommandLambda");
                        value->Add(modification);
                    },
                    [value, modification]
                    {
                        Logger::LogMessage("Rollback CommandLambda");
                        value->Subtract(modification);
                    });
            }
        case 1:
            return std::make_shared<AdditionCommand>(value);
        case 2:
        default:
            return std::make_shared<SubtractCommand>(value);
        }
    }
}

// TODO : Add WorkingValue checks to make sure all the modifications happen in the correct order.
TEST_CASE("Command Queue", "")
{
    std::shared_ptr<WorkingValue> value{std::make_shared<WorkingValue>()};
    CommandQueue queue{};
    REQUIRE_FALSE(queue.HasPendingCommand());
    REQUIRE_FALSE(queue.HasPendingRollbackCommand());
    REQUIRE(queue.GetCommandIndex() == 0);
    REQUIRE(queue.GetCommandQueueSize() == 0);

    SECTION("Add Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }

    SECTION("Execute Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 4);

        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 4);
        REQUIRE(queue.GetCommandQueueSize() == 4);
    }

    SECTION("Clear Command Queue")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 3);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ClearQueue();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 0);
    }

    SECTION("Prune Command Queue")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteNextCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 2);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        queue.ExecuteNextCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.PrunePendingCommands();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 2);
    }

    SECTION("Execute Rollback Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.RollbackCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value));
        queue.QueueCommand(CreateRandomCommand(value));
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        queue.ExecuteNextCommand();
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 3);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.RollbackCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.RollbackCommand();
        queue.RollbackCommand();
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }
}
