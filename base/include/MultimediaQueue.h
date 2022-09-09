#pragma once

#include "Module.h"


class MultimediaQueue;
class MultimediaQueueProps : public ModuleProps
{
public:
	MultimediaQueueProps() : ModuleProps()
	{
		maxQueueLength = 1000;
	}
	
	int maxQueueLength; // Length of multimedia queue
};

class State;

class MultimediaQueue : public Module {
public:
	MultimediaQueue(MultimediaQueueProps _props = MultimediaQueueProps());

	virtual ~MultimediaQueue() {
	}

	bool init();
	bool term();
	void getState(uint64_t Ts, uint64_t Te);
	void transitionTo(State* state);
	bool handleCommand(Command::CommandType type, frame_sp &frame);
	bool allowFrames(uint64_t&Ts, uint64_t &Te);
	bool setNext(boost::shared_ptr<Module> next, bool open = true, bool sieve = false);

protected:
	bool process(frame_container& frames);
	bool validateInputPins();
	bool validateOutputPins();
	bool validateInputOutputPins();
	//void addInputPin(framemetadata_sp& metadata, string& pinId);

public:
	boost::shared_ptr<State> mState;
	MultimediaQueueProps mProps;
};

class QueueClass;

class State {
public:
	boost::shared_ptr<QueueClass> queueObject;
	State() {}
	State(MultimediaQueueProps& _props) {}
	virtual ~State() {}
	typedef std::map<uint64_t, frame_container> mQueueMap;
	virtual bool handleExport(uint64_t &ts, uint64_t &te, bool& timeReset, mQueueMap& mQueue) { return true; }
	State* currentState;
	uint64_t startTime = 0;
	uint64_t endTime = 0;
	
	enum StateType 
	{
		IDLE,
		WAITING,
		EXPORT
	};

	State(StateType type_)
	{
		Type = type_;
	}
	StateType Type = StateType::IDLE;
};
