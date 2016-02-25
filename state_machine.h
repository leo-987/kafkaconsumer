#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include <string>
#include <map>

#include "network.h"
#include "node.h"

enum class Event {
	STARTUP,
	JOIN_REQUEST_WITH_EMPTY_CONSUMER_ID,
	HEARTBEAT,
	SENDER_STOPPED,
};

class StateMachine {
public:
	StateMachine(Network *network, std::map<std::string, Node*> &nodes);

	typedef int (StateMachine::*StateProc)(Event &event);
	StateProc current_state_;

	int Init();
	int Start();
	int Stop();

	// state functions
	int DiscoverCoordinator(Event &event);
	int PartOfGroup(Event &event);
	int StoppedConsumption(Event &event);

	Network *network_;
	std::map<std::string, Node*> &nodes_;
	int PushRequest(Node *node, Request *request);
	int PopResponse(Node *node, Response **response);

private:
	bool run_;
	Event event_;
	static void SignalHandler(int signal);
};

#endif
