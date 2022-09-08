#pragma once

#include "Module.h"


class MultimediaQueue;
class MultimediaQueueProps : public ModuleProps
{
public:
	MultimediaQueueProps() : ModuleProps()
	{
		maxQueueLength = 240000;
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
	void getState(int64_t Ts, int64_t Te);
	void transitionTo(State* state);
	bool handleCommand(Command::CommandType type, frame_sp &frame);
	bool allowFrames(const std::string &Ts, const std::string &Te);

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

class queueClass;

class State {
public:
	boost::shared_ptr<queueClass> queueObject;
	State() {}
	State(MultimediaQueueProps& _props) {}
	virtual ~State() {}
	typedef std::map<int64_t, frame_container> mQueueMap;
	virtual bool handleExport(int64_t ts, int64_t te, std::vector<frame_container>& frames, bool& timeReset, mQueueMap& mQueue) { return true; }
	State* currentState;
	int64_t startTime = 0;
	int64_t endTime = 0;
	
	enum StateType 
	{
		Idle,
		Waiting,
		Export
	};

	State(StateType type_)
	{
		Type = type_;
	}
	StateType Type = StateType::Idle;
};
