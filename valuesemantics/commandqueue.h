#pragma once

#include <memory>
#include <vector>

#include "valuesemantics/commandoperations.h"

namespace ValueSemantics
{
    class Command
    {
    public:
        template<class TCommand>
        Command(TCommand&& command)
            : m_Pimpl{std::make_unique<CommandModel<TCommand>>(std::move(command))}
        {
        }

        Command(const Command& other)
            : m_Pimpl{other.m_Pimpl->Clone()}
        {
        }

        Command& operator=(const Command& other)
        {
            if(this == &other)
                return *this;

            other.m_Pimpl->Clone().swap(m_Pimpl);
            return *this;
        }

        Command(Command&& other) = default;
        Command& operator=(Command&& other) = default;

        void Execute()
        {
            m_Pimpl->Execute();
        }

        void Rollback()
        {
            m_Pimpl->Rollback();
        }

    private:
        class CommandConcept
        {
        public:
            virtual ~CommandConcept() = default;
            virtual std::unique_ptr<CommandConcept> Clone() const = 0;
            virtual void Execute() = 0;
            virtual void Rollback() = 0;
        };

        template<class TCommand>
        class CommandModel final : public CommandConcept
        {
        public:
            CommandModel(TCommand&& command)
                : m_Command{std::move(command)}
            {
            }

            std::unique_ptr<CommandConcept> Clone() const override
            {
                return std::make_unique<CommandModel>(*this);
            }

            void Execute() override
            {
                ValueSemantics::Execute(m_Command);
            }

            void Rollback() override
            {
                ValueSemantics::Rollback(m_Command);
            }

            TCommand m_Command{};
        };

        std::unique_ptr<CommandConcept> m_Pimpl{};
    };

    class CommandQueue
    {
    public:
        /// HasPendingCommand() has to be true before calling
        void ExecuteCommand()
        {
            m_CommandQueue[m_CommandIndex].Execute();
            ++m_CommandIndex;
        }

        /// HasPendingRollbackCommand() has to be true before calling
        void RollbackCommand()
        {
            --m_CommandIndex;
            m_CommandQueue[m_CommandIndex].Rollback();
        }

        void ClearQueue()
        {
            m_CommandQueue.clear();
            m_CommandIndex = 0;
        }

        /// Removes any commands ahead of and including the current pending command.
        /// HasPendingCommand() has to be true before calling
        void ClearPendingCommands()
        {
            const auto itr{std::begin(m_CommandQueue) + m_CommandIndex};
            m_CommandQueue.erase(itr, std::end(m_CommandQueue));
        }

        [[nodiscard]] bool HasPendingCommand() const
        {
            return GetCommandQueueSize() > m_CommandIndex;
        }

        [[nodiscard]] bool HasPendingRollbackCommand() const
        {
            return m_CommandIndex != 0 && GetCommandQueueSize() >= m_CommandIndex;
        }

        void QueueCommand(Command&& command)
        {
            m_CommandQueue.push_back(std::move(command));
        }

        [[nodiscard]] uint32_t GetCommandIndex() const
        {
            return m_CommandIndex;
        }

        [[nodiscard]] uint32_t GetCommandQueueSize() const
        {
            return static_cast<uint32_t>(m_CommandQueue.size());
        }
    private:
        std::vector<Command> m_CommandQueue{};
        uint32_t m_CommandIndex{0};
    };
}
