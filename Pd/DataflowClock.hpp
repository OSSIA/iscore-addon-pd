#pragma once
#include <Engine/Executor/ClockManager/ClockManagerFactory.hpp>
#include <Engine/Executor/ClockManager/DefaultClockManager.hpp>

#include <portaudio.h>
namespace Process
{
class Cable;
}
namespace Dataflow
{
class DocumentPlugin;
class Clock final
    : public Engine::Execution::ClockManager
    , public Nano::Observer
{
    public:
        Clock(const Engine::Execution::Context& ctx);

        ~Clock();
private:
        void on_cableCreated(Process::Cable& c);
        void on_cableRemoved(const Process::Cable& c);

        void connectCable(Process::Cable& cable);

        // Clock interface
        void play_impl(
                const TimeVal& t,
                Engine::Execution::BaseScenarioElement&) override;
        void pause_impl(Engine::Execution::BaseScenarioElement&) override;
        void resume_impl(Engine::Execution::BaseScenarioElement&) override;
        void stop_impl(Engine::Execution::BaseScenarioElement&) override;
        bool paused() const override;

        Engine::Execution::DefaultClockManager m_default;
        Dataflow::DocumentPlugin& m_plug;
        Engine::Execution::BaseScenarioElement* m_cur{};
        bool m_paused{};
};

class ClockFactory final : public Engine::Execution::ClockManagerFactory
{
        ISCORE_CONCRETE("e9ae6dec-a10f-414f-9060-b21d15b5d58d")

        QString prettyName() const override;
        std::unique_ptr<Engine::Execution::ClockManager> make(
            const Engine::Execution::Context& ctx) override;

        std::function<ossia::time_value(const TimeVal&)>
        makeTimeFunction(const iscore::DocumentContext& ctx) const override;
        std::function<TimeVal(const ossia::time_value&)>
        makeReverseTimeFunction(const iscore::DocumentContext& ctx) const override;
};
}
