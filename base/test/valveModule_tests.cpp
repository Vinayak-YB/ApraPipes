#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "ExternalSourceModule.h"
#include "ExternalSinkModule.h"
#include "ValveModule.h"
#include "FrameMetadata.h"
#include "Frame.h"
#include "Logger.h"
#include "AIPExceptions.h"
#include "test_utils.h"

BOOST_AUTO_TEST_SUITE(valveModule_tests)

BOOST_AUTO_TEST_CASE(basic)
{
    unsigned int readDataSize = 0U;

    auto m1 = boost::shared_ptr<ExternalSourceModule>(new ExternalSourceModule());
	auto metadata = framemetadata_sp(new FrameMetadata(FrameMetadata::ENCODED_IMAGE));
	auto pinId = m1->addOutputPin(metadata);
	auto m2 = boost::shared_ptr<ValveModule>(new ValveModule(ValveModuleProps()));
	m1->setNext(m2);
    auto m3 = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule());
    m2->setNext(m3);

    BOOST_TEST(m1->init());
	BOOST_TEST(m2->init());

    auto m3Que = m3->getQue();

    auto frame = m1->makeFrame(readDataSize, pinId);
    frame_container frames;
	frames.insert(make_pair(pinId, frame));
	
    m1->send(frames);
	m2->step();
    m2->relay(m3,false);

    BOOST_TEST(m3Que->try_pop().size() == 0);

}
BOOST_AUTO_TEST_SUITE_END()