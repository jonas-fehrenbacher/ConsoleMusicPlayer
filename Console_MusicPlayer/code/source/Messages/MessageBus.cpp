#include "Message/MessageBus.hpp"
#include "Tools/Tool.hpp"

void core::MessageBus::send(int message)
{
	messages.push_back(message);
}

long long core::MessageBus::add(std::function<void(int)> receiver)
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
	messages.clear();
}