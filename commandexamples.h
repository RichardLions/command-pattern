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

    [[nodiscard]] static std::shared_ptr<Command> CreateModifyValueLambdaCommand(
        std::shared_ptr<WorkingValue> value, const int32_t valueModification)
    {
        return std::make_shared<LambdaCommand>(
            [value, valueModification]
            {
                value->ModifyValue(valueModification);
            },
            [value, valueModification]
            {
                value->ModifyValue(-valueModification);
            });
    }

    [[nodiscard]] static std::shared_ptr<Command> CreateModifyValueCommand(
        std::shared_ptr<WorkingValue> value, const int32_t valueModification)
    {
        return std::make_shared<ModifyValueCommand>(value, valueModification);
    }

    [[nodiscard]] static std::shared_ptr<Command> CreateRandomCommand(
        std::shared_ptr<WorkingValue> value, const int32_t valueModification = 0)
    {
        if(Random::RandomBool())
            return CreateModifyValueLambdaCommand(value, valueModification);

        return CreateModifyValueCommand(value, valueModification);
    }
}

TEST_CASE("Command Queue", "")
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
        queue.QueueCommand(CreateRandomCommand(value, 1));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 1);

        queue.QueueCommand(CreateRandomCommand(value, 2));
        queue.QueueCommand(CreateRandomCommand(value, 3));
        REQUIRE(value->GetValue() == 0);
        REQUIRE(queue.HasPendingCommand());
        REQUIRE_FALSE(queue.HasPendingRollbackCommand());
        REQUIRE(queue.GetCommandIndex() == 0);
        REQUIRE(queue.GetCommandQueueSize() == 3);
    }

    SECTION("Execute Commands")
    {
        queue.QueueCommand(CreateRandomCommand(value, 1));
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

        queue.QueueCommand(CreateRandomCommand(value, 2));
        queue.QueueCommand(CreateRandomCommand(value, 3));
        queue.QueueCommand(CreateRandomCommand(value, 4));
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
        queue.QueueCommand(CreateRandomCommand(value, 1));
        queue.QueueCommand(CreateRandomCommand(value, 2));
        queue.QueueCommand(CreateRandomCommand(value, 3));
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
        queue.QueueCommand(CreateRandomCommand(value, 1));
        queue.QueueCommand(CreateRandomCommand(value, 2));
        queue.QueueCommand(CreateRandomCommand(value, 3));
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

        queue.QueueCommand(CreateRandomCommand(value, 4));
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

        queue.QueueCommand(CreateRandomCommand(value, 5));
        queue.QueueCommand(CreateRandomCommand(value, 6));
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
        queue.QueueCommand(CreateRandomCommand(value, 1));
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

        queue.QueueCommand(CreateRandomCommand(value, 2));
        queue.QueueCommand(CreateRandomCommand(value, 3));
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
