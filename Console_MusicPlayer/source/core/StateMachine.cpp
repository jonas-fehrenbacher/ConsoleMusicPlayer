#include "core/StateMachine.hpp"
#include "core/Profiler.hpp"

core::State::State() :
	isDrawing(true),
	isUpdating(true)
{

}

core::State::~State()
{

}

void core::StateMachine::add(State* state)
{
	states.push_back(state);
	state->init();
}

void core::StateMachine::remove(State* state)
{
	state->terminate();
	states.erase(std::remove(states.begin(), states.end(), state), states.end());
}

void core::StateMachine::update()
{
	PROFILE_FUNC

	for (State* state : states) {
		if (state->isUpdating) {
			state->update();
		}
	}
}

void core::StateMachine::handleEvent()
{
	PROFILE_FUNC

	for (State* state : states) {
		if (state->isUpdating) {
			state->handleEvent();
		}
	}
}

void core::StateMachine::draw()
{
	PROFILE_FUNC

	for (State* state : states) {
		if (state->isDrawing) {
			state->draw();
		}
	}
}