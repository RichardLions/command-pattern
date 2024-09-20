#pragma once

#include <memory>

#include "command.h"

class CommandQueue final
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
        const uint32_t queueSize{GetCommandQueueSize()};
        return m_CommandIndex != 0 && queueSize >= m_CommandIndex;
    }

    void QueueCommand(std::shared_ptr<Command> command)
    {
        m_CommandQueue.push_back(command);
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
    std::vector<std::shared_ptr<Command>> m_CommandQueue{};
    uint32_t m_CommandIndex{0};
};
