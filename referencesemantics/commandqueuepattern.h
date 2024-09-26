#pragma once

#include <memory>
#include <utility>

#include "referencesemantics/commandpattern.h"

namespace ReferenceSemantics
{
    class CommandQueue
    {
    public:
        /// HasPendingCommand() has to be true before calling
        void ExecuteCommand()
        {
            m_CommandQueue[m_CommandIndex]->Execute();
            ++m_CommandIndex;
        }

        /// HasPendingRollbackCommand() has to be true before calling
        void RollbackCommand()
        {
            --m_CommandIndex;
            m_CommandQueue[m_CommandIndex]->Rollback();
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

        template<class CommandType>
        void QueueCommand(CommandType&& command)
        {
            m_CommandQueue.push_back(std::make_unique<CommandType>(std::forward<CommandType>(command)));
        }

        void QueueCommand(std::unique_ptr<Command>&& command)
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
        std::vector<std::unique_ptr<Command>> m_CommandQueue{};
        uint32_t m_CommandIndex{0};
    };
}
