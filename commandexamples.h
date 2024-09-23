#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "command.h"
#include "commandqueue.h"

namespace
{
    class WorkingValue
    {
    public:
        using ValueType = int32_t;

        [[nodiscard]] ValueType GetValue() const { return m_Value; }
        void ModifyValue(const ValueType modification)
        {
            m_Value += modification;
        }
    private:
        ValueType m_Value{0};
    };

    class ModifyValueCommand final : public Command
    {
    public:
        ModifyValueCommand(std::shared_ptr<WorkingValue> value, const int32_t valueModification)
            : m_Value{value}
            , m_Modification{valueModification}
        {
        }

        void Execute() override
        {
            m_Value->ModifyValue(m_Modification);
        }

        void Rollback() override
        {
            m_Value->ModifyValue(-m_Modification);
        }
    private:
        std::shared_ptr<WorkingValue> m_Value{};
        WorkingValue::ValueType m_Modification{};
    };

    [[nodiscard]] static LambdaCommand CreateLambdaCommand(
        std::shared_ptr<WorkingValue> value, const int32_t valueModification)
    {
        return LambdaCommand{
            [value, valueModification]
            {
                value->ModifyValue(valueModification);
            },
            [value, valueModification]
            {
                value->ModifyValue(-valueModification);
            }};
    }

    [[nodiscard]] static ModifyValueCommand CreateCommand(
        std::shared_ptr<WorkingValue> value, const int32_t valueModification)
    {
        return ModifyValueCommand{value, valueModification};
    }
}

TEST_CASE("Command Queue - Unit Tests")
{
    std::shared_ptr<WorkingValue> value{std::make_shared<WorkingValue>()};
    CommandQueue queue{};
    REQUIRE(value->GetValue() == 0);
    REQUIRE_FALSE(queue.HasPendingCommand());
    REQUIRE_FALSE(queue.HasPendingRollbackCommand());
    REQUIRE(queue.GetCommandIndex() == 0);
    REQUIRE(queue.GetCommandQueueSize() == 0);

    SECTION("Add Commands")
    {
        queue.QueueCommand(CreateCommand(value, 1));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateLambdaCommand(value, 2));
        queue.QueueCommand(CreateCommand(value, 3));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }

    SECTION("Execute Commands")
    {
        queue.QueueCommand(CreateLambdaCommand(value, 1));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.ExecuteCommand(); // +1
        REQUIRE(value->GetValue() == 1);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateLambdaCommand(value, 2));
        queue.QueueCommand(CreateCommand(value, 3));
        queue.QueueCommand(CreateLambdaCommand(value, 4));
        REQUIRE(value->GetValue() == 1);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 4);

        queue.ExecuteCommand(); // +2
        queue.ExecuteCommand(); // +3
        queue.ExecuteCommand(); // +4
        REQUIRE(value->GetValue() == 10);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 4);
        REQUIRE(queue.GetCommandQueueSize() == 4);
    }

    SECTION("Clear Command Queue")
    {
        queue.QueueCommand(CreateCommand(value, 1));
        queue.QueueCommand(CreateLambdaCommand(value, 2));
        queue.QueueCommand(CreateLambdaCommand(value, 3));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteCommand(); // +1
        queue.ExecuteCommand(); // +2
        queue.ExecuteCommand(); // +3
        REQUIRE(value->GetValue() == 6);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 3);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ClearQueue(); // Remove +1, +2, +3
        REQUIRE(value->GetValue() == 6);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 0);
    }

    SECTION("Clear Commands")
    {
        queue.QueueCommand(CreateLambdaCommand(value, 1));
        queue.QueueCommand(CreateCommand(value, 2));
        queue.QueueCommand(CreateLambdaCommand(value, 3));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteCommand(); // +1
        REQUIRE(value->GetValue() == 1);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ClearPendingCommands(); // Remove +2, +3
        REQUIRE(value->GetValue() == 1);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateCommand(value, 4));
        REQUIRE(value->GetValue() == 1);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 2);

        queue.ClearPendingCommands(); // Remove +4
        REQUIRE(value->GetValue() == 1);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateLambdaCommand(value, 5));
        queue.QueueCommand(CreateLambdaCommand(value, 6));
        queue.ExecuteCommand(); // +5
        REQUIRE(value->GetValue() == 6);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ClearPendingCommands(); // Remove +6
        REQUIRE(value->GetValue() == 6);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 2);

        queue.RollbackCommand(); // -5
        queue.RollbackCommand(); // -1
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 2);

        queue.ClearPendingCommands(); // Remove +1, +5
        REQUIRE(value->GetValue() == 0);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 0);
    }

    SECTION("Execute Rollback Commands")
    {
        queue.QueueCommand(CreateLambdaCommand(value, 1));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.ExecuteCommand(); // +1
        REQUIRE(value->GetValue() == 1);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 1);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.RollbackCommand(); // -1
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateCommand(value, 2));
        queue.QueueCommand(CreateLambdaCommand(value, 3));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.ExecuteCommand(); // +1
        queue.ExecuteCommand(); // +2
        queue.ExecuteCommand(); // +3
        REQUIRE(value->GetValue() == 6);
        REQUIRE_FALSE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 3);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.RollbackCommand(); // -3
        REQUIRE(value->GetValue() == 3);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 2);
        REQUIRE(queue.GetCommandQueueSize() == 3);

        queue.RollbackCommand(); // -2
        queue.RollbackCommand(); // -1
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }
}

TEST_CASE("Command Queue - Benchmarks")
{
    BENCHMARK("Benchmark")
    {
        constexpr uint32_t creationCount{10'000};
        std::shared_ptr<WorkingValue> value{std::make_shared<WorkingValue>()};
        CommandQueue queue{};

        for(uint32_t i{0}; i != creationCount; ++i)
        {
            queue.QueueCommand(CreateCommand(value, 0));
            queue.QueueCommand(CreateLambdaCommand(value, 0));
        }

        while(queue.HasPendingCommand())
        {
            queue.ExecuteCommand();
        }

        while(queue.HasPendingRollbackCommand())
        {
            queue.RollbackCommand();
        }
    };
}
