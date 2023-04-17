#include "State/StateMachine.hpp"

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
	for (State* state : states) {
		if (state->isUpdating) {
			state->update();
		}
	}
}

void core::StateMachine::handleEvent()
{
	for (State* state : states) {
		if (state->isUpdating) {
			state->handleEvent();
		}
	}
}

void core::StateMachine::draw()
{
	for (State* state : states) {
		if (state->isDrawing) {
			state->draw();
		}
	}
}