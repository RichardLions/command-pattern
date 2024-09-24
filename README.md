# Command Pattern

This pattern was inspired by [Game Programming Patterns](https://gameprogrammingpatterns.com/).

## When To Use

...

## Features

The command queue can:
* Queue Commands
* Execute Commands
* Rollback Commands
* Clear Commands

Example Usage:
```cpp
PopulateQueue(queue);

while(queue.HasPendingCommand())
{
    queue.ExecuteCommand();
}

while(queue.HasPendingRollbackCommand())
{
    queue.RollbackCommand();
}
```

Queuing a command after rolling back a command will add the new command to the back of the queue, behind the rolled back command. This may not be the desired effect, to solve this use the clear functions. Example:

```cpp
// Clear Pending
{
    queue.QueueCommand(commandA);
    queue.ExecuteCommand(); // Execute A
    queue.RollbackCommand(); // Rollback A

    queue.QueueCommand(commandB);
    queue.ExecuteCommand(); // Execute A
    queue.ExecuteCommand(); // Execute B

    queue.RollbackCommand(); // Rollback B
    queue.ClearPendingCommands() // Removes B

    queue.QueueCommand(commandC);
    queue.ExecuteCommand(); // Execute A
    queue.ExecuteCommand(); // Execute C
}

// Clear All
{
    queue.QueueCommand(commandA);
    queue.ExecuteCommand(); // Execute A
    queue.RollbackCommand(); // Rollback A
    queue.ClearQueue(); // Removes all commands

    queue.QueueCommand(commandB);
    queue.ExecuteCommand(); // Execute B
}
```

## Setup

This repository uses the .sln/.proj files created by Visual Studio 2022 Community Edition.
Using MSVC compiler, Preview version(C++23 Preview). 

### Catch2
The examples for how to use the pattern are written as Unit Tests.

Launching the program in Debug or Release will run the Unit Tests.

Alternative:
Installing the Test Adapter for Catch2 Visual Studio extension enables running the Unit Tests via the Test Explorer Window. Setup the Test Explorer to use the project's .runsettings file.

### vcpkg
This repository uses vcpkg in manifest mode for it's dependencies. To interact with vcpkg, open a Developer PowerShell (View -> Terminal).

To setup vcpkg, install it via the Visual Studio installer. To enable/disable it run these commands from the Developer PowerShell:
```
vcpkg integrate install
vcpkg integrate remove
```

To add additional dependencies run:
```
vcpkg add port [dependency name]
```

To update the version of a dependency modify the overrides section of vcpkg.json. 

To create a clean vcpkg.json and vcpkg-configuration.json file run:
```
vcpkg new --application
```

### TODO
- [x] Inheritance Implementation
- [x] Inheritance Implementation Unit Tests
- [ ] Type Erasure Implementation Example
- [ ] Type Erasure Implementation Example Unit Tests
- [ ] Benchmarking
