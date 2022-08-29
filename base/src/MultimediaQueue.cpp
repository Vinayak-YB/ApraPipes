#include <boost/foreach.hpp>
#include <map>
#include "FramesMuxer.h"
#include "Frame.h"
#include "MultimediaQueue.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "stdafx.h"

class DetailAbs 
{

public:
	DetailAbs(MultimediaQueueProps& _props) {}
	
	virtual ~DetailAbs()
	{

	}

	//virtual std::string addInputPin(std::string& pinId)
	//{
	//	return getMultimediaQueuePinId(pinId);
	//}

	virtual bool queue(frame_container& frames)
	{

		return true;
	}

	virtual bool get(frame_container& frames)
	{
		return false;
	}
	friend class State;
	typedef std::map<int64_t, frame_container> MultimediaQueueMap;
	MultimediaQueueMap mQueue;
protected:
	std::string getMultimediaQueuePinId(const std::string& pinId)
	{
		return pinId ;
	}
private:
	//MultimediaQueueProps mProps;
};

class Idle : public State {
public:
	void startExport(int64_t ts) 
	{
		//if (qObj.mQueue.size() )
	};
	void stopExport(int64_t te) {};
};

class Waiting : public State {
public:
	void startExport(int64_t ts) {};
	void stopExport(int64_t te) {};
};

class Export : public State {
public:
	void startExport(int64_t ts) {};
	void stopExport(int64_t te) {};
};
//Strategy begins here

class DetailJpeg : public DetailAbs
{

public:
	DetailJpeg(MultimediaQueueProps& _props) :DetailAbs(_props), maxQueueLength(_props.maxQueueLength) {}

	~DetailJpeg()
	{
		
	}

	bool queue(frame_container& frames)
	{
		// add all the frames to the que
		// store the most recent fIndex
		int64_t largestTimeStamp = 0;
		for (auto it = frames.cbegin(); it != frames.cend(); it++)
		{
			mQueue.insert({ it->second->timestamp, frames});
			if (largestTimeStamp < it->second->timestamp)
			{
				largestTimeStamp = it->second->timestamp;
			}
		}
		
		if (largestTimeStamp - mQueue.begin()->first > maxQueueLength)
		{
			mQueue.erase(mQueue.begin()->first);
		}

		return true;
	}

	bool get(frame_container& frames)
	{
		frames[DetailAbs::getMultimediaQueuePinId(mQueue.begin()->second.begin()->first)] = mQueue.begin()->second.begin()->second;
		return true;
	}

private:
	
	double maxQueueLength;
	int maxDelay;

};

// Methods

MultimediaQueue::MultimediaQueue(MultimediaQueueProps _props) :Module(TRANSFORM, "MultimediaQueue", _props),mProps(_props)
{
	mDetail.reset(new DetailJpeg(_props));
}

bool MultimediaQueue::validateInputPins()
{
	return true;
}

bool MultimediaQueue::validateOutputPins()
{
	return true;
}

bool MultimediaQueue::validateInputOutputPins()
{
	return Module::validateInputOutputPins();
}

bool MultimediaQueue::init()
{
	if (!Module::init())
	{
		return false;
	}


	return true;
}

bool MultimediaQueue::term()
{
	mDetail.reset();
	return Module::term();
}

void MultimediaQueue::transitionTo(State* state)
{
	/*if (state_ != nullptr)
		delete state_;*/
	state_ = state;
	state_->set_multimediaQueue(this); 
}

void MultimediaQueue::requestStart(int64_t ts)
{
	//state_ = (new Idle());
	state_->startExport(ts);
}

void MultimediaQueue::requestStop(int64_t te)
{
	state_->stopExport(te);
}

bool MultimediaQueue::handleCommand(Command::CommandType type, frame_sp &frame)
{
	if (type == Command::CommandType::MultimediaQueue)
	{
		MultimediaQueueCommand cmd;
		getCommand(cmd, frame);
		MultimediaQueue* multiQueObj = new MultimediaQueue(new Idle,mProps);
		multiQueObj->requestStart(cmd.startTime);
		multiQueObj->requestStop(cmd.endTime);
		return true;
	}
}

bool MultimediaQueue::allowFrames(int64_t Ts, int64_t Te)
{
	MultimediaQueueCommand cmd;
	cmd.startTime = Ts;
	cmd.endTime = Te;
	return queueCommand(cmd);
}

bool MultimediaQueue::process(frame_container& frames)
{

	mDetail->queue(frames);

	frame_container outFrames;
	mDetail->get(outFrames);
	
		send(outFrames);
		outFrames.clear();
	
	// LOG_ERROR << "Sending frames from multimedia queue";
	return true;
}

