#pragma once

#include "Module.h"

class MultimediaQueue;
class MultimediaQueueProps : public ModuleProps
{
public:
	MultimediaQueueProps() 
	{
		lowerWaterMark = 10000;
		upperWaterMark = 15000;
		isMapDelayInTime = true;
	}
	MultimediaQueueProps(double _lowerWaterMark = 10000,bool _isDelayTime = true)
	{
		lowerWaterMark = _lowerWaterMark;
		isMapDelayInTime = _isDelayTime;
		if (isMapDelayInTime == true)
		{
			upperWaterMark = lowerWaterMark + 5000;
		}
		else
		{
			upperWaterMark = lowerWaterMark + 5;
		}
	}
	
	double lowerWaterMark; // Length of multimedia queue in terms of time or number of frames
	double upperWaterMark; //Length of the multimedia queue when the next module queue is full
	bool isMapDelayInTime;  
};

class State;

class MultimediaQueue : public Module {
public:
	MultimediaQueue(MultimediaQueueProps _props);

	virtual ~MultimediaQueue() {
	}

	bool init();
	bool term();
	void getState(uint64_t Ts, uint64_t Te);
	bool handleCommand(Command::CommandType type, frame_sp &frame);
	bool allowFrames(uint64_t&Ts, uint64_t &Te);
	bool setNext(boost::shared_ptr<Module> next, bool open = true, bool sieve = false);
	void queueBoundaryTS(uint64_t& tOld, uint64_t& tNew);

protected:
	bool process(frame_container& frames);
	bool validateInputPins();
	bool validateOutputPins();
	bool validateInputOutputPins();

private:
	bool pushNext = true;
	bool reset = false;
	uint64_t startTimeSaved = 0;
	uint64_t endTimeSaved = 0;
public:
	boost::shared_ptr<State> mState ;
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
	virtual bool handleExport(uint64_t & queryStart, uint64_t & queryEnd, bool& timeReset, mQueueMap& mQueue) { return true; }
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
