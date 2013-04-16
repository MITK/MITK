//
// LoggerTest.cpp
//
// $Id$
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "LoggerTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "TestChannel.h"


using Poco::Logger;
using Poco::Channel;
using Poco::Message;
using Poco::AutoPtr;


LoggerTest::LoggerTest(const std::string& name): CppUnit::TestCase(name)
{
}


LoggerTest::~LoggerTest()
{
}


void LoggerTest::testLogger()
{
	AutoPtr<TestChannel> pChannel = new TestChannel;
	Logger& root = Logger::root();
	root.setChannel(pChannel.get());
	assert (root.getLevel() == Message::PRIO_INFORMATION);
	assert (root.is(Message::PRIO_INFORMATION));
	assert (root.fatal());
	assert (root.critical());
	assert (root.error());
	assert (root.warning());
	assert (root.notice());
	assert (root.information());
	assert (!root.debug());
	assert (!root.trace());
	
	root.information("Informational message");
	assert (pChannel->list().size() == 1);
	root.warning("Warning message");
	assert (pChannel->list().size() == 2);
	root.debug("Debug message");
	assert (pChannel->list().size() == 2);
	
	Logger& logger1 = Logger::get("Logger1");
	Logger& logger2 = Logger::get("Logger2");
	Logger& logger11 = Logger::get("Logger1.Logger1");
	Logger& logger12 = Logger::get("Logger1.Logger2");
	Logger& logger21 = Logger::get("Logger2.Logger1");
	Logger& logger22 = Logger::get("Logger2.Logger2");

	std::vector<std::string> loggers;
	Logger::names(loggers);
	assert (loggers.size() == 7);
	assert (loggers[0] == "");
	assert (loggers[1] == "Logger1");
	assert (loggers[2] == "Logger1.Logger1");
	assert (loggers[3] == "Logger1.Logger2");
	assert (loggers[4] == "Logger2");
	assert (loggers[5] == "Logger2.Logger1");
	assert (loggers[6] == "Logger2.Logger2");

	Logger::setLevel("Logger1", Message::PRIO_DEBUG);
	assert (logger1.is(Message::PRIO_DEBUG));
	assert (logger11.is(Message::PRIO_DEBUG));
	assert (logger12.is(Message::PRIO_DEBUG));
	assert (!logger2.is(Message::PRIO_DEBUG));
	assert (!logger21.is(Message::PRIO_DEBUG));
	assert (!logger22.is(Message::PRIO_DEBUG));
	assert (logger11.is(Message::PRIO_INFORMATION));
	assert (logger12.is(Message::PRIO_INFORMATION));
	assert (logger21.is(Message::PRIO_INFORMATION));
	assert (logger22.is(Message::PRIO_INFORMATION));
	
	Logger::setLevel("Logger2.Logger1", Message::PRIO_ERROR);
	assert (logger1.is(Message::PRIO_DEBUG));
	assert (logger11.is(Message::PRIO_DEBUG));
	assert (logger12.is(Message::PRIO_DEBUG));
	assert (!logger21.is(Message::PRIO_DEBUG));
	assert (!logger22.is(Message::PRIO_DEBUG));
	assert (logger11.is(Message::PRIO_INFORMATION));
	assert (logger12.is(Message::PRIO_INFORMATION));
	assert (logger21.is(Message::PRIO_ERROR));
	assert (logger22.is(Message::PRIO_INFORMATION));
	
	Logger::setLevel("", Message::PRIO_WARNING);
	assert (root.getLevel() == Message::PRIO_WARNING);
	assert (logger1.getLevel() == Message::PRIO_WARNING);
	assert (logger11.getLevel() == Message::PRIO_WARNING);
	assert (logger12.getLevel() == Message::PRIO_WARNING);
	assert (logger1.getLevel() == Message::PRIO_WARNING);
	assert (logger21.getLevel() == Message::PRIO_WARNING);
	assert (logger22.getLevel() == Message::PRIO_WARNING);
	
	AutoPtr<TestChannel> pChannel2 = new TestChannel;
	Logger::setChannel("Logger2", pChannel2.get());
	assert (pChannel  == root.getChannel());
	assert (pChannel  == logger1.getChannel());
	assert (pChannel  == logger11.getChannel());
	assert (pChannel  == logger12.getChannel());
	assert (pChannel2 == logger2.getChannel());
	assert (pChannel2 == logger21.getChannel());
	assert (pChannel2 == logger22.getChannel());
	
	root.setLevel(Message::PRIO_TRACE);
	pChannel->list().clear();
	root.trace("trace");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_TRACE);
	pChannel->list().clear();
	root.debug("debug");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_DEBUG);
	pChannel->list().clear();
	root.information("information");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_INFORMATION);
	pChannel->list().clear();
	root.notice("notice");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_NOTICE);
	pChannel->list().clear();
	root.warning("warning");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_WARNING);
	pChannel->list().clear();
	root.error("error");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_ERROR);
	pChannel->list().clear();
	root.critical("critical");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_CRITICAL);
	pChannel->list().clear();
	root.fatal("fatal");
	assert (pChannel->list().begin()->getPriority() == Message::PRIO_FATAL);
}


void LoggerTest::testFormat()
{
	std::string str = Logger::format("$0$1", "foo", "bar");
	assert (str == "foobar");
	str = Logger::format("foo$0", "bar");
	assert (str == "foobar");
	str = Logger::format("the amount is $$ $0", "100");
	assert (str == "the amount is $ 100");
	str = Logger::format("$0$1$2", "foo", "bar");
	assert (str == "foobar");
	str = Logger::format("$foo$0", "bar");
	assert (str == "$foobar");
	str = Logger::format("$0", "1");
	assert (str == "1");
	str = Logger::format("$0$1", "1", "2");
	assert (str == "12");
	str = Logger::format("$0$1$2", "1", "2", "3");
	assert (str == "123");
	str = Logger::format("$0$1$2$3", "1", "2", "3", "4");
	assert (str == "1234");
}


void LoggerTest::testDump()
{
	AutoPtr<TestChannel> pChannel = new TestChannel;
	Logger& root = Logger::root();
	root.setChannel(pChannel.get());
	root.setLevel(Message::PRIO_INFORMATION);
	
	char buffer1[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
	root.dump("test", buffer1, sizeof(buffer1));
	assert (pChannel->list().empty());
	
	root.setLevel(Message::PRIO_DEBUG);
	root.dump("test", buffer1, sizeof(buffer1));
	
	std::string msg = pChannel->list().begin()->getText();
	assert (msg == "test\n0000  00 01 02 03 04 05                                 ......");
	pChannel->clear();
	
	char buffer2[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};
	root.dump("", buffer2, sizeof(buffer2));
	msg = pChannel->list().begin()->getText();
	assert (msg == "0000  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ................");
	pChannel->clear();
	
	char buffer3[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x20, 0x41, 0x42, 0x1f, 0x7f, 0x7e
	};
	root.dump("", buffer3, sizeof(buffer3));
	msg = pChannel->list().begin()->getText();
	assert (msg == "0000  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ................\n"
	               "0010  20 41 42 1F 7F 7E                                  AB..~");
	pChannel->clear();
}


void LoggerTest::setUp()
{
	Logger::shutdown();
}


void LoggerTest::tearDown()
{
}


CppUnit::Test* LoggerTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("LoggerTest");

	CppUnit_addTest(pSuite, LoggerTest, testLogger);
	CppUnit_addTest(pSuite, LoggerTest, testFormat);
	CppUnit_addTest(pSuite, LoggerTest, testDump);

	return pSuite;
}
