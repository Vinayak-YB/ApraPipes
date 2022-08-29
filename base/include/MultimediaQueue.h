#pragma once

#include "Module.h"

class MultimediaQueue;
class MultimediaQueueProps : public ModuleProps
{
public:
	enum Strategy {
		JPEG,
		GOF,
		TimeStampStrategy
	};
public:
	MultimediaQueueProps() : ModuleProps()
	{
		maxQueueLength = 10000;
		strategy = JPEG;
		fIndexStrategyType = FIndexStrategy::FIndexStrategyType::NONE;
	}

	int maxQueueLength; // Length of multimedia queue
	Strategy strategy;
};

class State {

protected:
	MultimediaQueue* multimediaQueue_;

public:
	virtual ~State() {}

	void set_multimediaQueue(MultimediaQueue* multimediaQueue) {
		this->multimediaQueue_ = multimediaQueue;
	}
	virtual void startExport(int64_t ts) = 0;
	virtual void stopExport(int64_t te) = 0;

};

class MultimediaQueueStrategy;

class MultimediaQueue : public Module {
public:
	MultimediaQueue(MultimediaQueueProps _props = MultimediaQueueProps());
	MultimediaQueue(State* state) : state_(nullptr)
	{
		transitionTo(state);
	}
	virtual ~MultimediaQueue() {
		delete state_;
	}

	bool init();
	bool term();
	void transitionTo(State* state);
	void requestStart(int64_t Ts);
	void requestStop(int64_t Te);
	bool handleCommand(Command::CommandType type, frame_sp &frame);
	bool allowFrames(int64_t Ts, int64_t Te);

protected:
	bool process(frame_container& frames);
	bool validateInputPins();
	bool validateOutputPins();
	bool validateInputOutputPins();
	//void addInputPin(framemetadata_sp& metadata, string& pinId);
	

private:
	class Detail;
	boost::shared_ptr<MultimediaQueueStrategy> mDetail;
	State *state_;
};
