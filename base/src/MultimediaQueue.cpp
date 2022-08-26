#include <boost/foreach.hpp>
#include <map>
#include "FramesMuxer.h"
#include "Frame.h"
#include "MultimediaQueue.h"
#include "Logger.h"
#include "AIPExceptions.h"

class MultimediaQueueStrategy
{

public:
	MultimediaQueueStrategy(MultimediaQueueProps& _props) {}

	virtual ~MultimediaQueueStrategy()
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

protected:
	std::string getMultimediaQueuePinId(const std::string& pinId)
	{
		return pinId ;
	}
};

//Strategy begins here

class TimeStampStrategy : public MultimediaQueueStrategy
{

public:
	TimeStampStrategy(MultimediaQueueProps& _props) :MultimediaQueueStrategy(_props), maxQueueLength(_props.maxQueueLength) {}

	~TimeStampStrategy()
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
		frames[MultimediaQueueStrategy::getMultimediaQueuePinId(mQueue.begin()->second.begin()->first)] = mQueue.begin()->second.begin()->second;
		return true;
	}

private:

	typedef std::map<int64_t, frame_container> MultimediaQueue; // pinId and frame queue
	MultimediaQueue mQueue;
	double maxQueueLength;
	int maxDelay;

};

// Methods

MultimediaQueue::MultimediaQueue(MultimediaQueueProps _props) :Module(TRANSFORM, "MultimediaQueue", _props)
{
	mDetail.reset(new TimeStampStrategy(_props));
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
