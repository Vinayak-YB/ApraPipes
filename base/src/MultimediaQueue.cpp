#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>
#include "Frame.h"
#include "MultimediaQueue.h"
#include "Logger.h"
#include "stdafx.h"

class QueueClass 
{

public:
	QueueClass(MultimediaQueueProps& _props) {}
	
	~QueueClass()
	{}
	
	bool enqueue(frame_container& frames,double lowerWaterMark, double upperWaterMark, bool isMapDelayInTime, bool  pushNext)
	{	//	Here the frame_containers are inserted into the map
		uint64_t largestTimeStamp = 0;
		for (auto it = frames.cbegin(); it != frames.cend(); it++)
		{
			mQueue.insert({ it->second->timestamp, frames });
			if (largestTimeStamp < it->second->timestamp)
			{
				largestTimeStamp = it->second->timestamp;
			}
		}
		if (isMapDelayInTime) // If the lower and upper watermark are given in time
		{
			if ((largestTimeStamp - mQueue.begin()->first > lowerWaterMark) && (pushNext == true))
			{
				mQueue.erase(mQueue.begin()->first);
			}

			else if ((largestTimeStamp - mQueue.begin()->first > upperWaterMark) && (pushNext == false))
			{
				auto it = mQueue.begin();
				auto lastElement = mQueue.end();
				lastElement--;
				auto lastElementTimeStamp = lastElement->first;
				while (it != mQueue.end())
				{
					if ((lastElementTimeStamp - it->first) < lowerWaterMark)
					{
						break;
					}
					auto itr = it;
					++it;
					mQueue.erase(itr->first);
				}
				pushNext = true;
			};
		}
		else // If the lower and upper water mark are given in number of frames
		{
			if (( mQueue.size() > lowerWaterMark) && (pushNext == true))
			{
				mQueue.erase(mQueue.begin()->first);
			}

			else if ((mQueue.size() > upperWaterMark) && (pushNext == false))
			{
				auto it = mQueue.begin();
				while (it != mQueue.end())
				{
					if (mQueue.size() < lowerWaterMark)
					{
						break;
					}
					auto itr = it;
					++it;
					mQueue.erase(itr->first);
				}
				pushNext = true;
			};
		}
		return true;
	}
	typedef std::map<uint64_t, frame_container> MultimediaQueueMap;
	MultimediaQueueMap mQueue;
};

//State Design begins here

class Export : public State {
public:
	Export() : State(StateType::EXPORT) {}
	Export(uint64_t _startTime, uint64_t _endTime, boost::shared_ptr<QueueClass> queueObj) : State(StateType::EXPORT) {
		startTime = _startTime;
		endTime = _endTime;
		queueObject = queueObj;
	}

	bool handleExport(uint64_t &queryStart, uint64_t &queryEnd,bool & timeReset, mQueueMap& queueMap) override
	{	
		auto tOld = queueMap.begin()->first;
		auto temp = queueMap.end();
		temp--;
		auto tNew = temp->first;
		queryEnd = queryEnd - 1;
		if ((queryStart < tOld) && (queueMap.upper_bound(queryEnd)!=queueMap.end()))
		{
			queryStart = tOld;
			timeReset = true;
			return true;
		}
		else if ((queryEnd > tNew) && (queueMap.upper_bound(queryStart) != queueMap.end()))
		{
			if (tNew >= queryEnd)
			{
				timeReset = true;
			}
			queryEnd = tNew;

			return true;
		}
		else
		{
			timeReset = true;
			return true;
		}
	}
};

class Waiting : public State {
public:
	Waiting() : State(State::StateType::WAITING) {}
	Waiting(uint64_t _startTime, uint64_t _endTime, boost::shared_ptr<QueueClass> queueObj) : State(State::StateType::WAITING) {
		startTime = _startTime;
		endTime = _endTime;
		queueObject = queueObj;
	}
	bool handleExport(uint64_t & queryStart, uint64_t & queryEnd,bool& timeReset, mQueueMap& queueMap) override
	{
		BOOST_LOG_TRIVIAL (info) << "WAITING STATE : THE FRAMES ARE IN FUTURE!! WE ARE WAITING FOR THEM..";
		return true;
	}
};

class Idle : public State {
public:
	Idle() : State(StateType::IDLE) {}
	Idle(uint64_t _startTime, uint64_t _endTime, boost::shared_ptr<QueueClass> queueObj) : State(StateType::IDLE) {
		startTime = _startTime;
		endTime = _endTime;
		queueObject = queueObj;
	}
	bool handleExport(uint64_t & queryStart, uint64_t & queryEnd,bool& timeReset, mQueueMap& queueMap) override
	{
		//The code will not come here
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

	mState.reset(new Idle(mState->startTime,mState->endTime,mState->queueObject));

	return true;
}

bool MultimediaQueue::term()
{
	mState.reset();
	return Module::term();
}

void MultimediaQueue::queueBoundaryTS(uint64_t& tOld, uint64_t& tNew)
{
	auto queueMap = mState->queueObject->mQueue;
	tOld = queueMap.begin()->first;
	auto tempIT = queueMap.end();
	tempIT--;
	tNew = tempIT->first;
}

void MultimediaQueue::getState(uint64_t tStart, uint64_t tEnd)
{	
	uint64_t tOld, tNew = 0;
	queueBoundaryTS(tOld, tNew);

	//Checking conditions to determine the new state and transitions to it.
	
	if (tEnd < tOld)
	{
		BOOST_LOG_TRIVIAL(info) << "IDLE STATE : MAYBE THE FRAMES HAVE PASSED THE QUEUE";
		mState.reset(new Idle(mState->startTime, mState->endTime, mState->queueObject));
	}
	else if (tStart > tNew)
	{
		mState.reset(new Waiting(mState->startTime, mState->endTime, mState->queueObject));
	}
	else 
	{
		mState.reset(new Export(mState->startTime, mState->endTime, mState->queueObject));
	}

}


bool MultimediaQueue::handleCommand(Command::CommandType type, frame_sp &frame)
{
	if (type == Command::CommandType::MultimediaQueue)
	{
		MultimediaQueueCommand cmd;
		getCommand(cmd, frame);
		getState(cmd.startTime, cmd.endTime);
		mState->startTime = cmd.startTime;
		startTimeSaved = cmd.startTime;
		mState->endTime = cmd.endTime;
		endTimeSaved = cmd.endTime;
		bool reset = false;
		bool pushNext = true;
		if(mState->Type != mState->IDLE)
		{
			mState->handleExport(mState->startTime, mState->endTime, reset, mState->queueObject->mQueue);
			for (auto it = mState->queueObject->mQueue.begin(); it != mState->queueObject->mQueue.end(); it++)
			{
				if (((it->first) >= mState->startTime) && (((it->first) <= mState->endTime)))
				{
					if (isNextModuleQueFull())
					{
						pushNext = false;
					}
					else
					{
						send(it->second);
					}
				}
			}
		}
		if (mState->Type == mState->EXPORT)
		{	
			uint64_t tOld, tNew = 0;
			queueBoundaryTS(tOld, tNew);
			if (mState->endTime > tNew)
			{
				reset = false;
			}
			mState->startTime = tNew;
		}
		if (reset)
		{
			mState->startTime = 0;
			mState->endTime = 0;
			getState(mState->startTime, mState->endTime);
		}
		return true;
	}
}

bool MultimediaQueue::allowFrames(uint64_t &ts, uint64_t&te)
{
	if (mState->Type != mState->EXPORT)
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
	mState->queueObject->enqueue(frames, mProps.lowerWaterMark, mProps.upperWaterMark, mProps.isMapDelayInTime,pushNext);
	if (mState->Type == mState->EXPORT)
	{	
		uint64_t tOld, tNew = 0;
		queueBoundaryTS(tOld, tNew);
		mState->endTime = tNew;
	}

	if (mState->Type == mState->WAITING)
	{
		getState(mState->startTime, mState->endTime);
	}

	if (mState->Type != mState->IDLE)
	{
		mState->handleExport(mState->startTime, mState->endTime,reset,mState->queueObject->mQueue);
		for (auto it = mState->queueObject->mQueue.begin(); it != mState->queueObject->mQueue.end(); it++)
		{
			if (((it->first) >= (mState->startTime + 1)) && (((it->first) <= (endTimeSaved))))
			{
				if (isNextModuleQueFull())
				{
					pushNext = false;
				}
				else
				{
					send(it->second);
				}
			}
		}
	}
	if (mState->Type == mState->EXPORT)
	{	
		uint64_t tOld, tNew = 0;
		queueBoundaryTS(tOld, tNew);
		if (mState->endTime > tNew);
		{
			reset = false;
		}
		mState->startTime = tNew;
	}

	if (reset)
	{
		mState->startTime = 0;
		mState->endTime = 0;
		getState(mState->startTime, mState->endTime);
	}
	return true;
}

