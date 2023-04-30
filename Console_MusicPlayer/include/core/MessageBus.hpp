#pragma once

#include <vector>
#include <functional>

namespace core
{
	struct Message
	{
		int   id;
		void* userData;
	};

	enum MessageID
	{
		CONSOLE_RESIZE = 0,
		CUSTOM_MESSAGE // Use this to declare custom messages [ enum { First = core::MessageID::CUSTOM_MESSAGE, ... } ] 
	};

	class MessageBus
	{
	public:
		/* UserData will be automatically freed. */
		void send(int messageID, void* userData = nullptr);
		/* returns receiver id for remove() */
		long long add(std::function<void(Message)> receiver);
		void remove(long long receiverID);
		void update();
	private:
		struct Receiver
		{
			long long                    id; // neccessary because std::function has no operator==
			std::function<void(Message)> func;
		};

		std::vector<Message>  messages;
		std::vector<Receiver> receivers;
	};
}