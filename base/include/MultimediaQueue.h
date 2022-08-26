#pragma once

#include "Module.h"

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

class MultimediaQueueStrategy;

class MultimediaQueue : public Module {
public:

	MultimediaQueue(MultimediaQueueProps _props = MultimediaQueueProps());
	virtual ~MultimediaQueue() {}

	virtual bool init();
	virtual bool term();


protected:
	bool process(frame_container& frames);
	bool validateInputPins();
	bool validateOutputPins();
	bool validateInputOutputPins();
	//void addInputPin(framemetadata_sp& metadata, string& pinId);
	

private:
	class Detail;
	boost::shared_ptr<MultimediaQueueStrategy> mDetail;
};
