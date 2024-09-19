#pragma once

#include <memory>

#include "command.h"

class CommandQueue final
{
public:
    /// HasPendingCommand() has to be true before calling
    void ExecuteNextCommand()
    {
        m_CommandQueue[m_CommandIndex]->Execute();
        ++m_CommandIndex;
    }

    void ClearQueue()
    {
        m_CommandQueue.clear();
        m_CommandIndex = 0;
    }

    /// Removes any commands ahead of m_CommandIndex.
    /// HasPendingCommand() has to be true before calling
    void PrunePendingCommands()
    {
        const auto itr{std::begin(m_CommandQueue) + m_CommandIndex};
        m_CommandQueue.erase(itr, std::end(m_CommandQueue));
    }

    bool HasPendingCommand() const
    {
        return GetCommandQueueSize() > m_CommandIndex;
    }

    void QueueCommand(std::shared_ptr<Command> command)
    {
        m_CommandQueue.push_back(command);
    }

    uint32_t GetCommandIndex() const
    {
        return m_CommandIndex;
    }

    uint32_t GetCommandQueueSize() const
    {
        return static_cast<uint32_t>(m_CommandQueue.size());
    }
private:
    std::vector<std::shared_ptr<Command>> m_CommandQueue{};
    uint32_t m_CommandIndex{0};
};
