#include <iostream>
#include <string_view>
#include <random>
#include <functional>

namespace Utils
{
    static void LogMessage(const std::string_view message)
    {
        std::cout << message << "\n";
    }

    static bool RandomBool()
    {
        constexpr double_t probability{0.5};
        static std::default_random_engine generator{};
        static std::bernoulli_distribution distribution{probability};
        return distribution(generator);
    }
}

class Command
{
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
};

class CommandLambda final : public Command
{
public:
    using FunctionType = std::function<void()>;

    CommandLambda(FunctionType&& function)
        : m_Function{std::move(function)}
    {
    }

    void Execute() override
    {
        m_Function();
    }
private:
    FunctionType m_Function{};
};

class CommandA final : public Command
{
public:
    void Execute() override
    {
        Utils::LogMessage("Execute CommandA");
    }
};

class CommandB final : public Command
{
public:
    void Execute() override
    {
        Utils::LogMessage("Execute CommandB");
    }
};

static std::shared_ptr<Command> CreateRandomCommand()
{
    if(Utils::RandomBool())
        return std::make_shared<CommandLambda>([]{ Utils::LogMessage("Execute CommandLambda"); });
    else if(Utils::RandomBool())
        return std::make_shared<CommandA>();

    return std::make_shared<CommandB>();
}

int main()
{
    constexpr uint32_t limit{100};
    for(uint32_t i{0}; limit > i; ++i)
    {
        std::shared_ptr<Command> command{CreateRandomCommand()};
        command->Execute();
    }
}
