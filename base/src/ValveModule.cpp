#include "stdafx.h"
#include <boost/filesystem.hpp>
#include "ValveModule.h"
#include "Module.h"

ValveModule::ValveModule(ValveModuleProps _props)
	:Module(TRANSFORM, "ValveModule", _props)
{
    return true;
};

ValverModule::~ValveModule() {}

bool ValveModule::validateInputOutputPins()
{
    // TODO: 
    //check 1 - total numbe of input pins > 0
    // number of output pins = number of input pins
    if (getNumberOfInputPins() == 0)
	{		
		return false;
	}
    if (getNumberOfInputPins() != getNumberOfOutputPins())
	{		
		return false;
	}
    return true;
};

bool ValveModule::init() 
{
    if (!Module::init())
	{
		return false;
	}
    return true;
};

bool ValveModule::term() 
{
    return Module::term();
};

bool ValveModule::process(frame_container& frames) 
{
   send(frames);
   return true;
};

// handleCommand() - to discuss with Mradul 

// TODO
// write a UT for valve_module_tests.cpp - valve_tests
// basic -> relay true/false - frames should stop coming out - step()
// add a command (passthrough next x frames) - handleCommand
// UT for this command 