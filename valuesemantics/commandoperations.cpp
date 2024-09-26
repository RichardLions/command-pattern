#include "valuesemantics/commandoperations.h"
#include "valuesemantics/commands.h"

namespace ValueSemantics
{
    void Execute(ModifyValueCommand& command)
    {
        command.Execute();
    }

    void Rollback(ModifyValueCommand& command)
    {
        command.Rollback();
    }

    void Execute(LambdaCommand& command)
    {
        command.Execute();
    }

    void Rollback(LambdaCommand& command)
    {
        command.Rollback();
    }
}
