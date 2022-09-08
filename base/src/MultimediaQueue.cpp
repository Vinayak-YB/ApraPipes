#include <boost/foreach.hpp>
#include <map>
#include "FramesMuxer.h"
#include "Frame.h"
#include "MultimediaQueue.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "stdafx.h"
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

const std::string timeFormat{ "%Y%m%d %H:%M:%S" };

class QueueClass 
{

public:
	QueueClass(MultimediaQueueProps& _props) {}
	
	~QueueClass()
	{}

	bool enqueue(frame_container& frames)
	{
		uint64_t largestTimeStamp = 0;
		for (auto it = frames.cbegin(); it != frames.cend(); it++)
		{
			mQueue.insert({ it->second->timestamp, frames });
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
		frames[QueueClass::getMultimediaQueuePinId(mQueue.begin()->second.begin()->first)] = mQueue.begin()->second.begin()->second;
		return true;
	}
	void startExport(uint64_t ts) {};
	void stopExport(uint64_t te) {};
	typedef std::map<uint64_t, frame_container> MultimediaQueueMap;
	MultimediaQueueMap mQueue;
protected:

	double maxQueueLength = 10000;
	int maxDelay;
protected:
	std::string getMultimediaQueuePinId(const std::string& pinId)
	{
		return pinId ;
	}

};

//Strategy begins here

class Export : public State {
public:
	Export() : State(StateType::EXPORT) {}
	Export(mQueueMap mQueue)
	{
	}

	bool handleExport(uint64_t ts, uint64_t te, std::vector<frame_container>& frames, bool & timeReset, mQueueMap& queueMap) override
	{
		//auto queueMap = queueObject->mQueue;
		auto tOld = queueMap.begin()->first;
		auto Temp = queueMap.end();
		Temp--;
		auto tNew = Temp->first;

		if ((ts < tOld) && (queueMap.upper_bound(te)==queueMap.end()))
		{
			//To Do : Add the case where the frame container stop coming
			//Current assumption is that frame containers are coming at all time 
			for (auto it = queueMap.begin(); it != queueMap.lower_bound(te); it++)
			{
				frames.push_back(it->second);
			}
			timeReset = true;
			return true;
		}
		else if ((te > tNew) && (queueMap.upper_bound(ts) == queueMap.end()))
		{
			for (auto it = queueMap.upper_bound(ts); it != queueMap.end(); it++)
			{
				frames.push_back(it->second);
			}
			if (tNew >= te)
			{
				timeReset = true;
			}
			return true;
		}
		else
		{
			for (auto it = queueMap.upper_bound(ts); it != queueMap.lower_bound(te); it++)
			{
				frames.push_back(it->second);
			}
			timeReset = true;
			return true;
		}
	}
};

class Waiting : public State {
public:
	Waiting() : State(State::StateType::WAITING) {}
	bool handleExport(uint64_t ts, uint64_t te, std::vector<frame_container>& frames, bool& timeReset, mQueueMap& queueMap) override
	{
		auto tOld = queueMap.begin()->first;
		auto Temp = queueMap.end();
		Temp--;
		auto tNew = Temp->first;
		//The code will change here
		//The state will be waiting until we find ts in map, once found state will go to export
		if (tNew >= ts)
		{
			currentState = new (Export);
		}
		LOG_ERROR << "THE FRAMES ARE IN FUTURE!! WE ARE WAITING FOR THEM..";
		return true;
	}
};

class Idle : public State {
public:
	Idle() : State(StateType::IDLE) {}
	bool handleExport(uint64_t ts, uint64_t te, std::vector<frame_container>& frames, bool& timeReset, mQueueMap& queueMap) override
	{
		//The code will not come here

		frames.push_back(queueMap.begin()->second);
		return true;
	}
};

MultimediaQueue::MultimediaQueue(MultimediaQueueProps _props) :Module(TRANSFORM, "MultimediaQueue", _props),mProps(_props)
{
	mState.reset(new State(_props));
	mState->queueObject.reset(new QueueClass(_props));
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

bool MultimediaQueue::setNext(boost::shared_ptr<Module> next, bool open, bool sieve)
{
	return Module::setNext(next, open, false, sieve);
}
bool MultimediaQueue::init()
{
	if (!Module::init())
	{
		return false;
	}

	mState->currentState = new Idle;

	return true;
}

bool MultimediaQueue::term()
{
	mState.reset();
	return Module::term();
}

void MultimediaQueue::getState(uint64_t tStart, uint64_t tStop)
{
	auto queueMap = mState->queueObject->mQueue;
	auto tOld = queueMap.begin()->first;
	auto Temp = queueMap.end();
	Temp--;
	auto tNew = Temp->first;

	//Checking conditions to determine the new state to transition to.
	
	if (tStop < tOld)
	{
		LOG_ERROR << "THE FRAMES HAVE PASSED THE MAP";
		transitionTo(new Idle);
	}
	else if (tStart > tNew)
	{
		transitionTo(new Waiting);
	}
	else 
	{
		transitionTo(new Export);
	}

}

void MultimediaQueue::transitionTo(State* state)
{
	mState->currentState = state;
}

bool MultimediaQueue::handleCommand(Command::CommandType type, frame_sp &frame)
{
	if (type == Command::CommandType::MultimediaQueue)
	{
		MultimediaQueueCommand cmd;
		getCommand(cmd, frame);
		getState(cmd.startTime, cmd.endTime);
		mState->startTime = cmd.startTime;
		mState->endTime = cmd.endTime;

		return true;
	}
}

bool MultimediaQueue::allowFrames(uint64_t &ts, uint64_t&te)
{
	if (mState->currentState->Type != mState->EXPORT)
	{
		MultimediaQueueCommand cmd;
		cmd.startTime = ts;
		cmd.endTime = te;
		return queueCommand(cmd);
	};
	return true;
}

bool MultimediaQueue::process(frame_container& frames)
{
	
	mState->queueObject->enqueue(frames);
	frame_container outFrames;
	bool reset = false;
	std::vector<frame_container> frameVector;
	
	//getState
	

	if (mState->currentState->Type == mState->Type)
	{
		return true;
	}
	else
	{
		mState->currentState->handleExport(mState->startTime, mState->endTime, frameVector,reset,mState->queueObject->mQueue);
		for (frame_container& element : frameVector)
		{
			send(element);
		}
		if (frameVector.size())
		{
			auto it = frameVector.size();
			auto lastFrameContainer = frameVector[it - 1];
			auto frame = lastFrameContainer.begin();
			auto Ts = frame->second->timestamp;
			mState->startTime = Ts;
		}
	}

	if (reset)
	{
		mState->startTime = 0;
		mState->endTime = 0;
		getState(mState->startTime, mState->endTime);
	}
	return true;
}

