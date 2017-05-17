#include <cstdio>
#include <iostream>
#include <exception>
#include <stack>
#include <typeinfo>
#include "lsignal.h"

#define MethodName __func__

template<typename... Args>
std::string MakeString(const std::string& format, Args... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
	std::unique_ptr<char[]> buffer(new char[size]);
	snprintf(buffer.get(), size, format.c_str(), args...);

	return std::string(buffer.get(), buffer.get() + size - 1);
}

class TestRunner
{
public:
	static void StartTest(const char *testName)
	{
		m_testName = testName;
	}

	static void EndTest()
	{
		m_testName = "";
	}

	static const char* CurrentTest()
	{
		return m_testName;
	}

private:
	static const char* m_testName;

};

const char* TestRunner::m_testName;

class AssertHelper
{
public:
	static void VerifyValue(int expected, int actual, const char *message)
	{
		if (expected != actual)
		{
			throw std::exception(MakeString("\n\n  %s\n\n    Excepted: %d\n    Actual: %d", message, expected, actual).c_str());
		}
	}

	static void VerifyValue(bool expected, bool actual, const char *message)
	{
		if (expected != actual)
		{
			throw std::exception(MakeString("\n\n  %s\n\n    Excepted: %s\n    Actual: %s", message, expected ? "true" : "false", actual ? "true" : "false").c_str());
		}
	}

};

void ExecuteTest(std::function<void()> testMethod)
{
	try
	{
		testMethod();

		std::cout << "(*) Test " << TestRunner::CurrentTest() << " passed.";
	}
	catch (const std::exception &ex)
	{
		std::cout << "(!) Test " << TestRunner::CurrentTest() << " failed: " << ex.what() << "\n";
	}

	TestRunner::EndTest();

	std::cout << "\n";
}

struct SignalOwner : public lsignal::slot
{
};

void CreateSignal_SignalShouldBeUnlocked()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void()> sg;

	AssertHelper::VerifyValue(sg.is_locked(), false, "Signal should be unlocked.");
}

void LockSignal_SignalShouldBeLocked()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void()> sg;

	sg.set_lock(true);

	AssertHelper::VerifyValue(sg.is_locked(), true, "Signal should be locked.");
}

void UnlockSignal_SignalShouldBeUnlocked()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void()> sg;

	sg.set_lock(false);

	AssertHelper::VerifyValue(sg.is_locked(), false, "Signal should be unlocked.");
}

void CallSignalWithoutConnections_SignalShouldBeCalled()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void(int, bool)> sg;

	int paramOne = 7;
	bool paramTwo = true;

	sg(paramOne, paramTwo);
}

void CallSignalWithSingleConnection_SignalShouldBeCalled()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void(int, bool)> sg;

	int paramOne = 7;
	bool paramTwo = true;

	bool receiverCalled = false;

	std::function<void(int, bool)> receiver = [=, &receiverCalled](int p0, bool p1)
	{
		receiverCalled = true;

		AssertHelper::VerifyValue(p0, paramOne, "First parameter should be as expected.");
		AssertHelper::VerifyValue(p1, paramTwo, "Second parameter should be as expected.");
	};

	sg.connect(receiver);

	sg(paramOne, paramTwo);

	AssertHelper::VerifyValue(receiverCalled, true, "Receiver should be called.");
}

void CallSignalWithMultipleConnections_SignalShouldBeCalled()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void(int, bool)> sg;

	int paramOne = 7;
	bool paramTwo = true;

	unsigned char receiverCalledTimes = 0;

	std::function<void(int, bool)> receiver = [=, &receiverCalledTimes](int p0, bool p1)
	{
		++receiverCalledTimes;

		AssertHelper::VerifyValue(p0, paramOne, "First parameter should be as expected.");
		AssertHelper::VerifyValue(p1, paramTwo, "Second parameter should be as expected.");
	};

	sg.connect(receiver);
	sg.connect(receiver);

	sg(paramOne, paramTwo);

	AssertHelper::VerifyValue(receiverCalledTimes, 2, "Count of calls of receiver should be as expected.");
}

void SetSameOwnerToSeveralSignals_AllSignalsShouldBeNotifiedAboutOwnerDestruction()
{
	TestRunner::StartTest(MethodName);

	lsignal::signal<void()> sigOne;
	lsignal::signal<void()> sigTwo;

	bool receiverOneCalled = false;
	bool receiverTwoCalled = false;

	std::function<void()> receiverOne = [&receiverOneCalled]()
	{
		receiverOneCalled = true;
	};

	std::function<void()> receiverTwo = [&receiverTwoCalled]()
	{
		receiverTwoCalled = true;
	};

	{
		SignalOwner signalOwner;

		sigOne.connect(receiverOne, &signalOwner);
		sigTwo.connect(receiverTwo, &signalOwner);

		sigOne();
		sigTwo();

		AssertHelper::VerifyValue(receiverOneCalled, true, "First receiver should be called.");
		AssertHelper::VerifyValue(receiverTwoCalled, true, "Second receiver should be called.");
	}

	receiverOneCalled = false;
	receiverTwoCalled = false;

	sigOne();
	sigTwo();

	AssertHelper::VerifyValue(receiverOneCalled, false, "First receiver should not be called.");
	AssertHelper::VerifyValue(receiverTwoCalled, false, "Second receiver should not be called.");
}

void CreateSignalToSignalConnection_WhenFirstSignalIsDestroyed_SecondSignalShouldBeNotifed()
{
	TestRunner::StartTest(MethodName);

	bool receiverOneCalled = false;
	bool receiverTwoCalled = false;

	std::function<void()> receiverOne = [&receiverOneCalled]()
	{
		receiverOneCalled = true;
	};

	std::function<void()> receiverTwo = [&receiverTwoCalled]()
	{
		receiverTwoCalled = true;
	};

	lsignal::signal<void()> sigTwo;

	{
		lsignal::signal<void()> sigOne;

		sigOne.connect(receiverOne);
		sigTwo.connect(receiverTwo);

		sigOne.connect(&sigTwo);

		sigOne();

		AssertHelper::VerifyValue(receiverOneCalled, true, "First receiver should be called.");
		AssertHelper::VerifyValue(receiverTwoCalled, true, "Second receiver should be called.");
	}

	sigTwo();

	AssertHelper::VerifyValue(receiverTwoCalled, true, "Second receiver should be called.");
}

void CreateSignalToSignalConnection_WhenSecondSignalIsDestryoed_FirstSignalShouldBeNotifed()
{
	TestRunner::StartTest(MethodName);

	bool receiverOneCalled = false;
	bool receiverTwoCalled = false;

	std::function<void()> receiverOne = [&receiverOneCalled]()
	{
		receiverOneCalled = true;
	};

	std::function<void()> receiverTwo = [&receiverTwoCalled]()
	{
		receiverTwoCalled = true;
	};

	lsignal::signal<void()> sigOne;

	{
		lsignal::signal<void()> sigTwo;

		sigOne.connect(receiverOne);
		sigTwo.connect(receiverTwo);

		sigOne.connect(&sigTwo);

		sigOne();

		AssertHelper::VerifyValue(receiverOneCalled, true, "First receiver should be called.");
		AssertHelper::VerifyValue(receiverTwoCalled, true, "Second receiver should be called.");
	}

	receiverOneCalled = false;
	receiverTwoCalled = false;

	sigOne();

	AssertHelper::VerifyValue(receiverOneCalled, true, "First receiver should be called.");
	AssertHelper::VerifyValue(receiverTwoCalled, false, "Second receiver should not be called.");
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	ExecuteTest(CreateSignal_SignalShouldBeUnlocked);
	ExecuteTest(LockSignal_SignalShouldBeLocked);
	ExecuteTest(UnlockSignal_SignalShouldBeUnlocked);
	ExecuteTest(CallSignalWithSingleConnection_SignalShouldBeCalled);
	ExecuteTest(CallSignalWithMultipleConnections_SignalShouldBeCalled);
	ExecuteTest(CallSignalWithoutConnections_SignalShouldBeCalled);
	ExecuteTest(SetSameOwnerToSeveralSignals_AllSignalsShouldBeNotifiedAboutOwnerDestruction);
	ExecuteTest(CreateSignalToSignalConnection_WhenFirstSignalIsDestroyed_SecondSignalShouldBeNotifed);
	ExecuteTest(CreateSignalToSignalConnection_WhenSecondSignalIsDestryoed_FirstSignalShouldBeNotifed);

	std::cin.get();

	return 0;
}
