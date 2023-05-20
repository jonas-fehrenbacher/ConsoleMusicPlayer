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

core::ActiveState::ActiveState() :
	data(nullptr)
{

}

void core::ActiveState::set(State* state)
{
	if (data) {
		data->terminate();
	}
	data = state;
	if (state) {
		data->init();
	}
}

void core::ActiveState::update()
{
	PROFILE_FUNC;
	if (data->isUpdating) {
		data->update();
	}
}

void core::ActiveState::handleEvents()
{
	PROFILE_FUNC;
	if (data->isUpdating) {
		data->handleEvent();
	}
}

void core::ActiveState::draw()
{
	PROFILE_FUNC;
	if (data->isDrawing) {
		data->draw();
	}
}