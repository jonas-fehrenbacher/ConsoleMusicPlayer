#include "State/PlaylistEditorState.hpp"
#include "App.hpp"
#include "Tools/Tool.hpp"
#include "Message/Messages.hpp"
#include "Tools/InputDevice.hpp"

PlaylistEditorState::PlaylistEditorState(App* app) :
	app(app)
{
	
}

void PlaylistEditorState::init()
{
	// Set music directories:
	musicDirs = core::getConfigPathArr(core::getConfig("data/config.dat")[L"musicDirs"]);
}

void PlaylistEditorState::terminate()
{
	musicDirs.clear();
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
	for (auto& musicDir : musicDirs) {
		std::cout << core::ColoredStr(musicDir.string(), core::Color::White) << "\n";
		for (auto& it : fs::recursive_directory_iterator(musicDir)) {
			if (core::isSupportedAudioFile(it.path())) {
				std::cout << "\t" << it.path() << "\n";
			}
		}
	}
}