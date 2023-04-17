#pragma once

#include <vector>
#include <functional>

namespace core
{
	class MessageBus
	{
	public:
		void send(int message);
		/* returns receiver id for remove() */
		long long add(std::function<void(int)> receiver);
		void remove(long long receiverID);
		void update();
	private:
		struct Receiver
		{
			long long id; // neccessary because std::function has no operator==
			std::function<void(int)> func;
		};

		std::vector<int> messages;
		std::vector<Receiver> receivers;
	};
}