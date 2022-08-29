#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include"FileReaderModule.h"
#include "ExternalSourceModule.h"
#include "MultimediaQueue.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "test_utils.h"
#include <vector>
#include <ExternalSinkModule.h>
#include<Module.h>
#include "FrameContainerQueue.h"

BOOST_AUTO_TEST_SUITE(multimediaqueue_tests)

class SinkModuleProps : public ModuleProps
{
public:
    SinkModuleProps() : ModuleProps()
    {};
};

class SinkModule : public Module
{
public:
    SinkModule(SinkModuleProps props) : Module(SINK, "sinkModule", props)
    {};
    boost::shared_ptr<FrameContainerQueue> getQue() { return Module::getQue(); }
    frame_container pop()
    {
        return Module::pop();
    }

protected:
    bool process() {};
    bool validateOutputPins()
    {
        return true;
    }
    bool validateInputPins()
    {
        return true;
    }
};

BOOST_AUTO_TEST_CASE(basic)
{
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 24;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId  =  fileReader->addOutputPin(metadata);

	auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps()));
    fileReader->setNext(multiQueue, true, false);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));
    multiQueue->addOutputPin(metadata);
    multiQueue->setNext(sink, true, false);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    multiQueue->allowFrames(12, 50);
    multiQueue->step();
    for (int i = 0; i < 21; i++)
    {
        fileReader->step();
        multiQueue->step();
        if (i == 19)
        {
            auto pop = sinkQueue->pop();
        }
    }
    //auto popFrames = sink->
   
    BOOST_TEST(sinkQueue->size()==1);
}

BOOST_AUTO_TEST_SUITE_END()