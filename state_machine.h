#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include <string>
#include <map>

#include "network.h"
#include "node.h"


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

private:
	bool run_;
	Event event_;
};

#endif
