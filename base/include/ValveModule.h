#pragma once 
#include "Module.h"

using namespace std;

class ValveModuleProps : public ModuleProps
{
public:
ValveModuleProps() : ModuleProps(){
    return true;
}
};

class ValveModule : public Module
{
public:
    public:
	ValveModule(ValveModuleProps _props);
	virtual ~ValveModule();
	bool init();
	bool term();
protected:
	bool process(frame_container& frames);
	bool validateInputOutputPins();
}
