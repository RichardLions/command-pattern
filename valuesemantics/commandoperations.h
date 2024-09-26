#pragma once

namespace ValueSemantics
{
    class ModifyValueCommand;

    void Execute(ModifyValueCommand& command);
    void Rollback(ModifyValueCommand& command);

    class LambdaCommand;

    void Execute(LambdaCommand& command);
    void Rollback(LambdaCommand& command);
}
