#include "Keymap.hpp"
#include "core/SmallTools.hpp"

intern std::wstring actionToStr(Keymap::Action action)
{
	if (action == Keymap::Action::Exit) return L"Exit";
	if (action == Keymap::Action::Back) return L"Back";
	if (action == Keymap::Action::PrevTrack) return L"PrevTrack";
	if (action == Keymap::Action::NextTrack) return L"NextTrack";
	if (action == Keymap::Action::LockInput) return L"LockInput";
	if (action == Keymap::Action::SelectNavBar) return L"SelectNavBar";
	if (action == Keymap::Action::PlayPause) return L"PlayPause";
	if (action == Keymap::Action::Shuffle) return L"Shuffle";
	if (action == Keymap::Action::IncreaseVolume) return L"IncreaseVolume";
	if (action == Keymap::Action::DecreaseVolume) return L"DecreaseVolume";
	if (action == Keymap::Action::Repeat) return L"Repeat";
	if (action == Keymap::Action::TrackSkipBackward) return L"TrackSkipBackward";
	if (action == Keymap::Action::TrackSkipForward) return L"TrackSkipForward";
	if (action == Keymap::Action::KeyInfo) return L"KeyInfo";
	if (action == Keymap::Action::Select) return L"Select";
	__debugbreak();
	return L"";
}

void Keymap::init()
{
	std::map<std::wstring, std::wstring> config = core::getConfig("data/keymap.properties");
	for (int i = 0; i < data.size(); ++i) {
		data[i] = (core::inputDevice::Key)stoi(config.at(actionToStr((Action)i)));
	}

	symbolMap[(int)core::inputDevice::Key::A] = "A";
	symbolMap[(int)core::inputDevice::Key::B] = "B";
	symbolMap[(int)core::inputDevice::Key::C] = "C";
	symbolMap[(int)core::inputDevice::Key::D] = "D";
	symbolMap[(int)core::inputDevice::Key::E] = "E";
	symbolMap[(int)core::inputDevice::Key::F] = "F";
	symbolMap[(int)core::inputDevice::Key::G] = "G";
	symbolMap[(int)core::inputDevice::Key::H] = "H";
	symbolMap[(int)core::inputDevice::Key::I] = "I";
	symbolMap[(int)core::inputDevice::Key::J] = "J";
	symbolMap[(int)core::inputDevice::Key::K] = "K";
	symbolMap[(int)core::inputDevice::Key::L] = "L";
	symbolMap[(int)core::inputDevice::Key::M] = "M";
	symbolMap[(int)core::inputDevice::Key::N] = "N";
	symbolMap[(int)core::inputDevice::Key::O] = "O";
	symbolMap[(int)core::inputDevice::Key::P] = "P";
	symbolMap[(int)core::inputDevice::Key::Q] = "Q";
	symbolMap[(int)core::inputDevice::Key::R] = "R";
	symbolMap[(int)core::inputDevice::Key::S] = "S";
	symbolMap[(int)core::inputDevice::Key::T] = "T";
	symbolMap[(int)core::inputDevice::Key::U] = "U";
	symbolMap[(int)core::inputDevice::Key::V] = "V";
	symbolMap[(int)core::inputDevice::Key::W] = "W";
	symbolMap[(int)core::inputDevice::Key::X] = "X";
	symbolMap[(int)core::inputDevice::Key::Y] = "Y";
	symbolMap[(int)core::inputDevice::Key::Z] = "Z";
	symbolMap[(int)core::inputDevice::Key::Num0] = "0";
	symbolMap[(int)core::inputDevice::Key::Num1] = "1";
	symbolMap[(int)core::inputDevice::Key::Num2] = "2";
	symbolMap[(int)core::inputDevice::Key::Num3] = "3";
	symbolMap[(int)core::inputDevice::Key::Num4] = "4";
	symbolMap[(int)core::inputDevice::Key::Num5] = "5";
	symbolMap[(int)core::inputDevice::Key::Num6] = "6";
	symbolMap[(int)core::inputDevice::Key::Num7] = "7";
	symbolMap[(int)core::inputDevice::Key::Num8] = "8";
	symbolMap[(int)core::inputDevice::Key::Num9] = "9";
	symbolMap[(int)core::inputDevice::Key::Escape] = "Esc";
	symbolMap[(int)core::inputDevice::Key::LControl] = "LControl";
	symbolMap[(int)core::inputDevice::Key::LShift] = "LShift";
	symbolMap[(int)core::inputDevice::Key::LAlt] = "LAlt";
	symbolMap[(int)core::inputDevice::Key::LSystem] = "LSystem";
	symbolMap[(int)core::inputDevice::Key::RControl] = "RControl";
	symbolMap[(int)core::inputDevice::Key::RShift] = "RShift";
	symbolMap[(int)core::inputDevice::Key::RAlt] = "RAlt";
	symbolMap[(int)core::inputDevice::Key::RSystem] = "RSystem";
	symbolMap[(int)core::inputDevice::Key::Menu] = "Menu";
	symbolMap[(int)core::inputDevice::Key::LBracket] = "[";
	symbolMap[(int)core::inputDevice::Key::RBracket] = "]";
	symbolMap[(int)core::inputDevice::Key::Semicolon] = ";";
	symbolMap[(int)core::inputDevice::Key::Comma] = ",";
	symbolMap[(int)core::inputDevice::Key::Period] = ".";
	symbolMap[(int)core::inputDevice::Key::Quote] = "'";
	symbolMap[(int)core::inputDevice::Key::Slash] = "/";
	symbolMap[(int)core::inputDevice::Key::Backslash] = "\\";
	symbolMap[(int)core::inputDevice::Key::Tilde] = "~";
	symbolMap[(int)core::inputDevice::Key::Equal] = "=";
	symbolMap[(int)core::inputDevice::Key::Plus] = "+";
	symbolMap[(int)core::inputDevice::Key::Minus] = "-";
	symbolMap[(int)core::inputDevice::Key::Space] = "Space";
	symbolMap[(int)core::inputDevice::Key::Enter] = "Enter";
	symbolMap[(int)core::inputDevice::Key::Backspace] = "Backspace";
	symbolMap[(int)core::inputDevice::Key::Tab] = "Tab";
	symbolMap[(int)core::inputDevice::Key::PageUp] = "PageUp";
	symbolMap[(int)core::inputDevice::Key::PageDown] = "PageDown";
	symbolMap[(int)core::inputDevice::Key::End] = "End";
	symbolMap[(int)core::inputDevice::Key::Home] = "Home";
	symbolMap[(int)core::inputDevice::Key::Insert] = "Insert";
	symbolMap[(int)core::inputDevice::Key::Delete] = "Delete";
	symbolMap[(int)core::inputDevice::Key::Left] = core::uc::leftwardsArrow;
	symbolMap[(int)core::inputDevice::Key::Right] = core::uc::rightwardsArrow;
	symbolMap[(int)core::inputDevice::Key::Up] = core::uc::upwardsArrow;
	symbolMap[(int)core::inputDevice::Key::Down] = core::uc::downwardsArrow;
	symbolMap[(int)core::inputDevice::Key::NumpadAdd] = "Numpad+";
	symbolMap[(int)core::inputDevice::Key::NumpadSubtract] = "Numpad-";
	symbolMap[(int)core::inputDevice::Key::NumpadMultiply] = "Numpad*";
	symbolMap[(int)core::inputDevice::Key::NumpadDivide] = "Numpad/";
	symbolMap[(int)core::inputDevice::Key::NumpadSeperator] = "Numpad,";
	symbolMap[(int)core::inputDevice::Key::NumpadDecimal] = "Numpad.";
	symbolMap[(int)core::inputDevice::Key::Numpad0] = "Numpad0";
	symbolMap[(int)core::inputDevice::Key::Numpad1] = "Numpad1";
	symbolMap[(int)core::inputDevice::Key::Numpad2] = "Numpad2";
	symbolMap[(int)core::inputDevice::Key::Numpad3] = "Numpad3";
	symbolMap[(int)core::inputDevice::Key::Numpad4] = "Numpad4";
	symbolMap[(int)core::inputDevice::Key::Numpad5] = "Numpad5";
	symbolMap[(int)core::inputDevice::Key::Numpad6] = "Numpad6";
	symbolMap[(int)core::inputDevice::Key::Numpad7] = "Numpad7";
	symbolMap[(int)core::inputDevice::Key::Numpad8] = "Numpad8";
	symbolMap[(int)core::inputDevice::Key::Numpad9] = "Numpad9";
	symbolMap[(int)core::inputDevice::Key::F1] = "F1";
	symbolMap[(int)core::inputDevice::Key::F2] = "F2";
	symbolMap[(int)core::inputDevice::Key::F3] = "F3";
	symbolMap[(int)core::inputDevice::Key::F4] = "F4";
	symbolMap[(int)core::inputDevice::Key::F5] = "F5";
	symbolMap[(int)core::inputDevice::Key::F6] = "F6";
	symbolMap[(int)core::inputDevice::Key::F7] = "F7";
	symbolMap[(int)core::inputDevice::Key::F8] = "F8";
	symbolMap[(int)core::inputDevice::Key::F9] = "F9";
	symbolMap[(int)core::inputDevice::Key::F10] = "F10";
	symbolMap[(int)core::inputDevice::Key::F11] = "F11";
	symbolMap[(int)core::inputDevice::Key::F12] = "F12";
	symbolMap[(int)core::inputDevice::Key::F13] = "F13";
	symbolMap[(int)core::inputDevice::Key::F14] = "F14";
	symbolMap[(int)core::inputDevice::Key::F15] = "F15";
	symbolMap[(int)core::inputDevice::Key::Pause] = "Pause";
	symbolMap[(int)core::inputDevice::Key::MediaNextTrack] = "MediaNextTrack";
	symbolMap[(int)core::inputDevice::Key::MediaPrevTrack] = "MediaPrevTrack";
	symbolMap[(int)core::inputDevice::Key::MediaStop] = "MediaStop";
	symbolMap[(int)core::inputDevice::Key::MediaPlayPause] = "MediaPlayPause";
}

Keymap::Key Keymap::get(Keymap::Action action)
{
	Key key;
	key.key    = data[(int)action];
	key.symbol = symbolMap[(int)key.key];
	
	return key;
}