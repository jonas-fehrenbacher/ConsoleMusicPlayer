#include "core/MessageBus.hpp"
#include "core/SmallTools.hpp"

void core::MessageBus::send(int messageID, void* userData /*= nullptr*/)
{
	messages.push_back(Message{ messageID, userData });
}

long long core::MessageBus::add(std::function<void(Message)> receiver)
{
	receivers.push_back({ getUUID(), receiver });
	return receivers.back().id;
}

void core::MessageBus::remove(long long receiverID)
{
	receivers.erase(std::remove_if(receivers.begin(), receivers.end(), 
		[receiverID](Receiver receiver) { return receiver.id == receiverID; }), receivers.end());
}

void core::MessageBus::update()
{
	for (auto& message : messages) {
		for (auto& receiver : receivers) {
			receiver.func(message);
		}
	}
	// I'm freeing everything afterwards, because two messages could share the same userData.
	// For example you could send in an message callback a message with the received userData. 
	for (auto& message : messages) {
		if (message.userData) {
			delete message.userData;
			message.userData = nullptr;
		}
	}
	messages.clear();
}