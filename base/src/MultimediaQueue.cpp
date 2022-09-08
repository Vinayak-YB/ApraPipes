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

class queueClass 
{

public:
	queueClass(MultimediaQueueProps& _props) {}
	
	~queueClass()
	{}

	bool queue(frame_container& frames)
	{
		int64_t largestTimeStamp = 0;
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
		frames[queueClass::getMultimediaQueuePinId(mQueue.begin()->second.begin()->first)] = mQueue.begin()->second.begin()->second;
		return true;
	}
	void startExport(int64_t ts) {};
	void stopExport(int64_t te) {};
	typedef std::map<int64_t, frame_container> MultimediaQueueMap;
	MultimediaQueueMap mQueue;

protected:

	double maxQueueLength = 100000;
	int maxDelay;
protected:
	std::string getMultimediaQueuePinId(const std::string& pinId)
	{
		return pinId ;
	}
private:
	//MultimediaQueueProps mProps;
};

class Waiting : public State {
public:
	Waiting() : State(State::StateType::Waiting) {}
	bool handleExport(int64_t ts, int64_t te, std::vector<frame_container>& frames, bool& timeReset, mQueueMap& queueMap) override
	{
		//auto queueMap = queueObject->mQueue;
		auto tOld = queueMap.begin()->first;
		auto Temp = queueMap.end();
		Temp--;
		auto tNew = Temp->first;
		//The code will change here
		//auto queueMap = queueObject->mQueue;
		frames.push_back(queueMap.begin()->second);
		return true;
	}
};

class Export : public State {
public:
	Export() : State(StateType::Export) {}
	Export(mQueueMap mQueue)
	{
	}

	bool handleExport(int64_t ts, int64_t te, std::vector<frame_container>& frames, bool & timeReset, mQueueMap& queueMap) override
	{
		//auto queueMap = queueObject->mQueue;
		auto tOld = queueMap.begin()->first;
		auto Temp = queueMap.end();
		Temp--;
		auto tNew = Temp->first;

		if ((ts < tOld) && (queueMap.upper_bound(te)!=queueMap.end()))
		{
			//To Do : Add the case where the frame container stop coming
			//Current assumption is that frame containers are coming at all time 
			for (auto it = queueMap.begin(); it != queueMap.lower_bound(te); it++)
			{
				frames.push_back(it->second);
			}
	
			return true;
		}
		else if ((te > tNew) && (queueMap.upper_bound(ts) != queueMap.end()))
		{
			for (auto it = queueMap.upper_bound(ts); it != queueMap.end(); it++)
			{
				frames.push_back(it->second);
			}
			return true;
		}
		else
		{
			for (auto it = queueMap.upper_bound(ts); it != queueMap.lower_bound(te); it++)
			{
				frames.push_back(it->second);
			}
			return true;
		}
	}
};
//Strategy begins here



class Idle : public State {
public:
	Idle() : State(StateType::Idle) {}
	bool handleExport(int64_t ts, int64_t te, std::vector<frame_container>& frames, bool& timeReset, mQueueMap& queueMap) override
	{
		//The code will change here
		//auto queueMap = queueObject->mQueue;
		frames.push_back(queueMap.begin()->second);
		return true;
	}
};

MultimediaQueue::MultimediaQueue(MultimediaQueueProps _props) :Module(TRANSFORM, "MultimediaQueue", _props),mProps(_props)
{
	mState.reset(new State(_props));
	mState->queueObject.reset(new queueClass(_props));
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

	mState->currentState = new Idle;

	return true;
}

bool MultimediaQueue::term()
{
	mState.reset();
	return Module::term();
}

void MultimediaQueue::getState(int64_t tStart, int64_t tStop)
{
	auto queueMap = mState->queueObject->mQueue;
	auto tOld = queueMap.begin()->first;
	auto Temp = queueMap.end();
	Temp--;
	auto tNew = Temp->first;

	//Check conditions and determine the new state to transition to.
	
	if (tStop < tOld)
	{
		LOG_ERROR << "THE FRAMES HAVE PASSED THE MAP";
		//mState->currentState = new (Export);
		//mState->currentState = new (Idle);
		transitionTo(new Export);
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
		//auto statePass = new Idle();
	/*	MultimediaQueue* multiQueObj = new MultimediaQueue(mProps);
		multiQueObj->requestStart(cmd.startTime);
		multiQueObj->requestStop(cmd.endTime);*/

		return true;
	}
}

int64_t getTimeStamp(const std::string& timeString)
{
	std::istringstream m_istream{ timeString };
	std::tm m_tm{ 0 };
	std::time_t m_timet{ 0 };
	m_istream >> std::get_time(&m_tm, timeFormat.c_str());
	m_timet = std::mktime(&m_tm);
	m_timet *= 1000; // convert to milliseconds
	return m_timet;
}

bool MultimediaQueue::allowFrames(const std::string &ts, const std::string &te)
{
	int64_t timeStart = getTimeStamp(ts);
	int64_t timeStop = getTimeStamp(te);
	MultimediaQueueCommand cmd;
	cmd.startTime = timeStart;
	cmd.endTime = timeStop;
	return queueCommand(cmd);
}

bool MultimediaQueue::process(frame_container& frames)
{
	
	mState->queueObject->queue(frames);
	frame_container outFrames;
	bool reset = false;
	std::vector<frame_container> frameVector;
	
	//getState
	State::StateType idleState = State::StateType::Idle;

	if (mState->currentState->Type == idleState)
	{
		mState->queueObject->get(outFrames);
		send(outFrames);
	}
	else
	{
		mState->currentState->handleExport(mState->startTime, mState->endTime, frameVector,reset,mState->queueObject->mQueue);
		for (frame_container& element : frameVector)
		{
			send(element);
		}
	}

	if (reset == true)
	{
		mState->startTime = 0;
		mState->endTime = 0;
		getState(mState->startTime, mState->endTime);
	}
	return true;
}

