#include "State/PlaylistEditorState.hpp"
#include "Messages.hpp"
#include "App.hpp"
#include "core/SmallTools.hpp"
#include "core/InputDevice.hpp"
#include "core/ColoredStr.hpp"

PlaylistEditorState::PlaylistEditorState(App* app) :
	app(app)
{
	
}

void PlaylistEditorState::init()
{
	
}

void PlaylistEditorState::terminate()
{
	
}

void PlaylistEditorState::update()
{

}

void PlaylistEditorState::handleEvent()
{
	//B-Key:
	if (core::inputDevice::isKeyPressed('B'))
		app->messageBus.send(Message::PlaylistEditorState_Finished);
}

void PlaylistEditorState::draw()
{
	for (auto& musicDir : app->musicDirs) {
		std::cout << core::ColoredStr(musicDir.string(), core::Color::White) << "\n";
		for (auto& it : fs::recursive_directory_iterator(musicDir)) {
			if (core::isSupportedAudioFile(it.path())) {
				std::cout << "\t" << it.path() << "\n";
			}
		}
	}
}