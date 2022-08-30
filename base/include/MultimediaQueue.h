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

class DetailAbs;

class State {

protected:
	

public:
	MultimediaQueue* multimediaQueue_;

	virtual ~State() {}

	void set_multimediaQueue(MultimediaQueue* multimediaQueue)
	{
		multimediaQueue_ = multimediaQueue;
	}
	boost::shared_ptr<DetailAbs> mDetail = multimediaQueue_->mDetail;
	virtual void startExport(int64_t ts) = 0;
	virtual void stopExport(int64_t te) = 0;

};



class MultimediaQueue : public Module {
public:
	MultimediaQueue(MultimediaQueueProps _props = MultimediaQueueProps());

	MultimediaQueue(State* state, MultimediaQueueProps _props = MultimediaQueueProps()) : Module(TRANSFORM, "MultimediaQueue",_props)
	{
		transitionTo(state);
	}
	virtual ~MultimediaQueue() {
		//delete state_;
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
	friend class State;
	boost::shared_ptr<DetailAbs> mDetail;
	State *state_ ;
	MultimediaQueueProps mProps;
};
