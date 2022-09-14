#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include"FileReaderModule.h"
#include "MultimediaQueue.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "test_utils.h"
#include <ExternalSinkModule.h>
#include "Module.h"
#include "FrameContainerQueue.h"
#include <time.h>
#include <chrono>


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

//The multimedia queue takes two arguements queue length (time or no of frames) and bool isDelayInTime. If the bool is true then the length is taken in time else in frames.

BOOST_AUTO_TEST_CASE(export_state)
{
    //In this case both the timestamps (query startTime and query endTime) are in the queue and we pass all the frames requested.
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId  =  fileReader->addOutputPin(metadata);

	auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10, false)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

	BOOST_TEST(fileReader->init());
	BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    uint64_t startTime = now - 9000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now - 4000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();
 
    for (int i = 0; i < 5; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    BOOST_TEST(sinkQueue->size()==5);
}

BOOST_AUTO_TEST_CASE(idle_state)
{
    //In this case both the timestamps (query startTime and endTime) are in the past of the oldest timestamp of queue so state is Idle all the time
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId = fileReader->addOutputPin(metadata);

    auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10000, true)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

    BOOST_TEST(fileReader->init());
    BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t startTime = now - 30000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now - 25000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();

    for (int i = 0; i < 5; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    BOOST_TEST(sinkQueue->size() == 0);
}

BOOST_AUTO_TEST_CASE(wait_state)
{
    //In this case both the timestamps (query startTime and endTime) are in the future of the latest timestamp of queue so state is Waiting all the time
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId = fileReader->addOutputPin(metadata);

    auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10000, true)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

    BOOST_TEST(fileReader->init());
    BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t startTime = now + 5000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now + 10000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();

    for (int i = 0; i < 5; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    BOOST_TEST(sinkQueue->size() == 0);
}

BOOST_AUTO_TEST_CASE(wait_to_export_state)
{
    //In this case initially we are in wait state then go to export after sometime.
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId = fileReader->addOutputPin(metadata);

    auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10000, true)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

    BOOST_TEST(fileReader->init());
    BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t startTime = now + 3000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now + 11000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();

    for (int i = 0; i < 6; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    BOOST_TEST(sinkQueue->size() == 3);
}

BOOST_AUTO_TEST_CASE(future_export)
{
    //In this case the timestamp of startTime is in the queue while endTime is in future so we start with export and continue to stay in export as frames are passed.
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId = fileReader->addOutputPin(metadata);

    auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10000, true)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

    BOOST_TEST(fileReader->init());
    BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t startTime = now - 3000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now + 5000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();

    for (int i = 0; i < 15; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    BOOST_TEST(sinkQueue->size() == 8);
}

BOOST_AUTO_TEST_CASE(nextQueue_full)
{
    //In this case, while the frames are being sent to next module the queue of next module must becme full 
    std::string inFolderPath = "./data/Raw_YUV420_640x360";
    auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1, 4 * 1024 * 1024);
    fileReaderProps.fps = 1;
    fileReaderProps.readLoop = true;
    auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps));
    auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
    auto pinId = fileReader->addOutputPin(metadata);

    auto multiQueue = boost::shared_ptr<MultimediaQueue>(new MultimediaQueue(MultimediaQueueProps(10, false)));
    fileReader->setNext(multiQueue);
    auto sink = boost::shared_ptr<SinkModule>(new SinkModule(SinkModuleProps()));

    multiQueue->setNext(sink);

    BOOST_TEST(fileReader->init());
    BOOST_TEST(multiQueue->init());
    BOOST_TEST(sink->init());
    auto sinkQueue = sink->getQue();
    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
    }
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t startTime = now - 10000;
    startTime = (startTime / 1000) * 1000;
    uint64_t endTime = now + 25000;
    endTime = (endTime / 1000) * 1000;
    multiQueue->allowFrames(startTime, endTime);
    multiQueue->step();

    for (int i = 0; i < 20; i++)
    {
        fileReader->step();
        multiQueue->step();
        if (i == 12)
        {
            sinkQueue->pop();
        }
    }
   
    BOOST_TEST(sinkQueue->size() == 20);
}

BOOST_AUTO_TEST_SUITE_END()