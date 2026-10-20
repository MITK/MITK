/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>
#include <mitkSharedBuffer.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkRGBPixel.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <unistd.h>
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{
  std::string ReadTestModule()
  {
    std::ifstream file(MITK_PYTHON_WORKER_TEST_MODULE);
    if (!file)
      CPPUNIT_FAIL(std::string("Cannot open the test module ") + MITK_PYTHON_WORKER_TEST_MODULE);

    std::ostringstream source;
    source << file.rdbuf();
    return source.str();
  }

  std::unique_ptr<mitk::PythonWorker> StartWorker(const std::vector<mitk::PythonWorker::Module>& modules)
  {
    return std::make_unique<mitk::PythonWorker>(mitk::PythonHelper::GetExecutablePath(), modules, "test worker");
  }

  std::unique_ptr<mitk::PythonWorker> StartWorker()
  {
    return StartWorker({ { "mitk_worker_test", ReadTestModule() } });
  }

  template <typename T>
  std::vector<T> Values(const mitk::SharedBuffer& buffer)
  {
    const auto* data = static_cast<const T*>(buffer.GetData());
    return std::vector<T>(data, data + buffer.GetSize() / sizeof(T));
  }

  bool WaitUntilStopped(const mitk::PythonWorker& worker)
  {
    for (int i = 0; i < 100 && worker.IsRunning(); ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return !worker.IsRunning();
  }

  bool WaitUntilGone(int pid)
  {
#if defined(_WIN32)
    auto* process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));

    if (process == nullptr)
      return true;

    const auto result = WaitForSingleObject(process, 10000);
    CloseHandle(process);

    return result == WAIT_OBJECT_0;
#else
    for (int i = 0; i < 200; ++i)
    {
      if (kill(pid, 0) == -1 && errno == ESRCH)
        return true;

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return false;
#endif
  }

  std::size_t CountOpenHandles()
  {
#if defined(_WIN32)
    DWORD count = 0;
    GetProcessHandleCount(GetCurrentProcess(), &count);
    return count;
#else
    std::size_t count = 0;

    for ([[maybe_unused]] const auto& entry : std::filesystem::directory_iterator("/dev/fd"))
      ++count;

    return count;
#endif
  }

  std::string ToUtf8(const std::filesystem::path& path)
  {
    const auto text = path.u8string();
    return std::string(text.begin(), text.end());
  }
}

class mitkPythonWorkerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonWorkerTestSuite);
  // First: the leftovers are removed once per process, before the first buffer.
  MITK_TEST(TestLeftoverSegmentsAreRemoved);
  MITK_TEST(TestRoundTrip);
  MITK_TEST(TestPythonError);
  MITK_TEST(TestModuleError);
  MITK_TEST(TestModulesImportEachOther);
  MITK_TEST(TestOutputDoesNotDisturbProtocol);
  MITK_TEST(TestUnreadableReplyEndsWorker);
  MITK_TEST(TestLargeResultIsRefused);
  MITK_TEST(TestOversizedMessageEndsWorker);
  MITK_TEST(TestEndlessErrorLine);
  MITK_TEST(TestBuffersInBothDirections);
  MITK_TEST(TestImageBuffers);
  MITK_TEST(TestImageOutlivesBuffer);
  MITK_TEST(TestKeptBuffer);
  MITK_TEST(TestProgressAndCancel);
  MITK_TEST(TestCallsFromAnotherThread);
  MITK_TEST(TestThrowingHookEndsWorker);
  MITK_TEST(TestExit);
  MITK_TEST(TestCrash);
  MITK_TEST(TestAccessViolation);
  MITK_TEST(TestForkedChildDoesNotHideExit);
  MITK_TEST(TestOrphanExits);
  MITK_TEST(TestWorkersAreIndependent);
  MITK_TEST(TestDestroyingBusyWorkerDoesNotWait);
  MITK_TEST(TestDescendantsEndWithWorker);
  MITK_TEST(TestDescendantsEndWithExitedWorker);
  MITK_TEST(TestDescendantsEndWithMitk);
  MITK_TEST(TestNoHandleLeaks);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestLeftoverSegmentsAreRemoved()
  {
#if defined(__linux__)
    // Named after a process ID above the largest one Linux hands out.
    const std::filesystem::path leftover = "/dev/shm/mitk7fffff00_0_0";
    std::ofstream(leftover) << "left behind";
    CPPUNIT_ASSERT(std::filesystem::exists(leftover));

    mitk::SharedBuffer buffer(mitk::MakeScalarPixelType<unsigned char>(), { 1 });

    CPPUNIT_ASSERT_MESSAGE("The segment of an ended process should be removed", !std::filesystem::exists(leftover));
#endif
  }

  void TestRoundTrip()
  {
    auto worker = StartWorker();
    const auto python = mitk::PythonHelper::GetExecutablePath();

    CPPUNIT_ASSERT(worker->IsRunning());
    CPPUNIT_ASSERT(mitk::PythonWorker::IsAnyRunning(python));

    const nlohmann::json params = { { "number", 42 }, { "text", "Gr\xC3\xBC\xC3\x9F" "e\nzwei Zeilen" }, { "list", { 1.5, true, nullptr } } };
    CPPUNIT_ASSERT_EQUAL(params, worker->Call("mitk_worker_test.echo", params));

    worker.reset();
    CPPUNIT_ASSERT(!mitk::PythonWorker::IsAnyRunning(python));
  }

  void TestPythonError()
  {
    auto worker = StartWorker();

    try
    {
      worker->Call("mitk_worker_test.fail", { { "message", "expected failure" } });
      CPPUNIT_FAIL("A Python exception should be reported");
    }
    catch (const mitk::PythonWorkerError& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("ValueError"), e.GetPythonType());
      CPPUNIT_ASSERT_EQUAL(std::string("ValueError: expected failure"), std::string(e.GetDescription()));
      CPPUNIT_ASSERT_MESSAGE("The traceback should show the source line", e.GetTraceback().find("raise ValueError(message)") != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE("The traceback should show the lines of the runtime as well", e.GetTraceback().find("return function(**kwargs)") != std::string::npos);
    }

    // _kept is None, so calling it would raise a TypeError if private names
    // were not refused by the runtime.
    try
    {
      worker->Call("mitk_worker_test._kept");
      CPPUNIT_FAIL("A private name should be refused");
    }
    catch (const mitk::PythonWorkerError& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("AttributeError"), e.GetPythonType());
    }

    CPPUNIT_ASSERT_THROW(worker->Call("os.getcwd"), mitk::PythonWorkerError);

    // A result that is not JSON must be an error, not a missing response.
    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.not_a_number"), mitk::PythonWorkerError);

    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestModuleError()
  {
    auto worker = StartWorker({ { "broken", "def broken(:\n    pass\n" } });

    try
    {
      worker->Call("broken.broken");
      CPPUNIT_FAIL("The syntax error should be reported");
    }
    catch (const mitk::PythonWorkerError& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("SyntaxError"), e.GetPythonType());
    }
  }

  void TestModulesImportEachOther()
  {
    auto worker = StartWorker({
      { "first_module", "def value():\n    return 21\n" },
      { "second_module", "import first_module\n\ndef doubled():\n    return 2 * first_module.value()\n" }
    });

    CPPUNIT_ASSERT_EQUAL(42, worker->Call("second_module.doubled").get<int>());
  }

  void TestOutputDoesNotDisturbProtocol()
  {
    auto worker = StartWorker();

    CPPUNIT_ASSERT_EQUAL(nlohmann::json("clean"), worker->Call("mitk_worker_test.pollute"));
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestUnreadableReplyEndsWorker()
  {
    auto worker = StartWorker();

    // An escaped lone surrogate would be invalid JSON for MITK.
    CPPUNIT_ASSERT_EQUAL(nlohmann::json("a?b"), worker->Call("mitk_worker_test.lone_surrogate"));

    const auto start = std::chrono::steady_clock::now();
    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.write_unreadable"), mitk::PythonWorkerExitedException);
    CPPUNIT_ASSERT(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
    CPPUNIT_ASSERT(!worker->IsRunning());
  }

  void TestLargeResultIsRefused()
  {
    auto worker = StartWorker();

    try
    {
      worker->Call("mitk_worker_test.large_result", { { "size", 17 * 1024 * 1024 } });
      CPPUNIT_FAIL("A result larger than a message may be should be refused");
    }
    catch (const mitk::PythonWorkerError& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("ValueError"), e.GetPythonType());
    }

    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestOversizedMessageEndsWorker()
  {
    auto worker = StartWorker();
    const auto start = std::chrono::steady_clock::now();

    try
    {
      worker->Call("mitk_worker_test.write_oversized", { { "size", 17 * 1024 * 1024 } });
      CPPUNIT_FAIL("An oversized message should end the worker");
    }
    catch (const mitk::PythonWorkerExitedException& e)
    {
      const std::string description = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE(description, description.find("sent a message larger than") != std::string::npos);
    }

    CPPUNIT_ASSERT(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
    CPPUNIT_ASSERT(!worker->IsRunning());
  }

  void TestEndlessErrorLine()
  {
    auto worker = StartWorker();

    // Longer than the part of a line that MITK collects at once.
    CPPUNIT_ASSERT_EQUAL(nlohmann::json("done"), worker->Call("mitk_worker_test.write_endless_error_line", { { "size", 1024 * 1024 } }));
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestBuffersInBothDirections()
  {
    auto worker = StartWorker();

    mitk::SharedBuffer source(mitk::MakeScalarPixelType<std::uint16_t>(), { 3, 4, 5 });
    auto* sourceData = static_cast<std::uint16_t*>(source.GetData());
    std::iota(sourceData, sourceData + 60, std::uint16_t{ 1 });

    const auto description = worker->Call("mitk_worker_test.describe", {}, { { "array", &source } });
    CPPUNIT_ASSERT_EQUAL(std::string("u2"), description["dtype"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(nlohmann::json({ 3, 4, 5 }), description["shape"]);
    CPPUNIT_ASSERT_EQUAL(1830.0, description["sum"].get<double>());

    mitk::SharedBuffer target(mitk::MakeScalarPixelType<float>(), { 7, 9 });
    worker->Call("mitk_worker_test.fill", { { "value", 2.5 } }, { { "target", &target } });

    for (const auto value : Values<float>(target))
      CPPUNIT_ASSERT_EQUAL(2.5f, value);

    // Refused before anything is sent. On Linux and macOS the worker would
    // fail to attach to the withdrawn name and report a Python error instead.
    try
    {
      worker->Call("mitk_worker_test.describe", {}, { { "array", &source } });
      CPPUNIT_FAIL("A buffer can be passed to one call only");
    }
    catch (const mitk::PythonWorkerException& e)
    {
      CPPUNIT_ASSERT_MESSAGE("The call should be refused by MITK, not by the worker",
        dynamic_cast<const mitk::PythonWorkerError*>(&e) == nullptr &&
        dynamic_cast<const mitk::PythonWorkerExitedException*>(&e) == nullptr);
    }
  }

  void TestImageBuffers()
  {
    auto worker = StartWorker();

    auto image = mitk::Image::New();
    const unsigned int dimensions[] = { 4, 3, 2 };
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);
    image->AllocateZeroedVolume();

    {
      mitk::ImageWriteAccessor accessor(image);
      auto* data = static_cast<short*>(accessor.GetData());
      std::iota(data, data + 24, short{ -10 });
    }

    auto copy = mitk::SharedBuffer::CopyOf(image);
    const auto description = worker->Call("mitk_worker_test.describe", {}, { { "array", copy.get() } });
    CPPUNIT_ASSERT_EQUAL(std::string("i2"), description["dtype"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(nlohmann::json({ 2, 3, 4 }), description["shape"]);
    CPPUNIT_ASSERT_EQUAL(36.0, description["sum"].get<double>());

    mitk::SharedBuffer result(mitk::MakeScalarPixelType<unsigned char>(), { 2, 3, 4 });
    auto resultImage = result.AsImage(*image->GetTimeGeometry());
    worker->Call("mitk_worker_test.fill", { { "value", 3 } }, { { "target", &result } });

    mitk::ImageReadAccessor accessor(resultImage);
    const auto* data = static_cast<const unsigned char*>(accessor.GetData());
    CPPUNIT_ASSERT_EQUAL(static_cast<const void*>(result.GetData()), accessor.GetData());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(3), data[23]);

    mitk::SharedBuffer mismatch(mitk::MakeScalarPixelType<unsigned char>(), { 2, 4, 3 });
    CPPUNIT_ASSERT_THROW(mismatch.AsImage(*image->GetTimeGeometry()), mitk::Exception);

    // A single-slice 3D image copies as (1, y, x), but a geometry of that
    // extent initializes a 2D image.
    auto slice = mitk::Image::New();
    const unsigned int sliceDimensions[] = { 4, 3, 1 };
    slice->Initialize(mitk::MakeScalarPixelType<short>(), 3, sliceDimensions);
    slice->AllocateZeroedVolume();

    auto sliceCopy = mitk::SharedBuffer::CopyOf(slice);
    CPPUNIT_ASSERT(sliceCopy->GetShape() == std::vector<std::size_t>({ 1, 3, 4 }));

    mitk::SharedBuffer sliceResult(mitk::MakeScalarPixelType<unsigned char>(), sliceCopy->GetShape());
    auto sliceImage = sliceResult.AsImage(*slice->GetTimeGeometry());
    CPPUNIT_ASSERT_EQUAL(2u, sliceImage->GetDimension());
    CPPUNIT_ASSERT_EQUAL(4u, sliceImage->GetDimension(0));
    CPPUNIT_ASSERT_EQUAL(3u, sliceImage->GetDimension(1));

    // The 2D image uses the memory of the buffer as well.
    worker->Call("mitk_worker_test.fill", { { "value", 9 } }, { { "target", &sliceResult } });
    mitk::ImageReadAccessor sliceAccessor(sliceImage);
    const auto* sliceData = static_cast<const unsigned char*>(sliceAccessor.GetData());
    CPPUNIT_ASSERT_EQUAL(static_cast<const void*>(sliceResult.GetData()), static_cast<const void*>(sliceData));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(9), sliceData[11]);

    mitk::SharedBuffer sliceMismatch(mitk::MakeScalarPixelType<unsigned char>(), { 1, 4, 3 });
    CPPUNIT_ASSERT_THROW(sliceMismatch.AsImage(*slice->GetTimeGeometry()), mitk::Exception);

    // A later time step is copied, not the first one.
    auto dynamic = mitk::Image::New();
    const unsigned int dynamicDimensions[] = { 4, 3, 2, 2 };
    dynamic->Initialize(mitk::MakeScalarPixelType<short>(), 4, dynamicDimensions);

    for (int timeStep = 0; timeStep < 2; ++timeStep)
    {
      const std::vector<short> values(24, static_cast<short>(timeStep + 1));
      dynamic->SetVolume(values.data(), timeStep);
    }

    auto secondTimeStep = mitk::SharedBuffer::CopyOf(dynamic, 1);
    CPPUNIT_ASSERT(secondTimeStep->GetShape() == std::vector<std::size_t>({ 2, 3, 4 }));
    CPPUNIT_ASSERT_EQUAL(48.0, worker->Call("mitk_worker_test.describe", {}, { { "array", secondTimeStep.get() } })["sum"].get<double>());

    using RGBImage = itk::Image<itk::RGBPixel<unsigned char>, 3>;
    CPPUNIT_ASSERT_THROW(mitk::SharedBuffer(mitk::MakePixelType<RGBImage>(), { 2, 2 }), mitk::Exception);
  }

  void TestImageOutlivesBuffer()
  {
    auto geometryImage = mitk::Image::New();
    const unsigned int dimensions[] = { 4, 3, 2 };
    geometryImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    mitk::Image::Pointer image;

    {
      mitk::SharedBuffer buffer(mitk::MakeScalarPixelType<unsigned char>(), { 2, 3, 4 });
      std::fill_n(static_cast<unsigned char*>(buffer.GetData()), buffer.GetSize(), static_cast<unsigned char>(5));
      image = buffer.AsImage(*geometryImage->GetTimeGeometry());
    }

    // The image keeps the memory mapped after the buffer is gone.
    mitk::ImageReadAccessor accessor(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(5), static_cast<const unsigned char*>(accessor.GetData())[23]);
  }

  void TestKeptBuffer()
  {
    auto worker = StartWorker();

    mitk::SharedBuffer buffer(mitk::MakeScalarPixelType<int>(), { 10 });
    worker->Call("mitk_worker_test.keep", {}, { { "array", &buffer } });

    // The name is withdrawn by now, but the mapping of the worker stays valid.
    worker->Call("mitk_worker_test.write_kept", { { "value", 7 } });

    for (const auto value : Values<int>(buffer))
      CPPUNIT_ASSERT_EQUAL(7, value);

    static_cast<int*>(buffer.GetData())[0] = 100;
    CPPUNIT_ASSERT_EQUAL(163.0, worker->Call("mitk_worker_test.sum_kept").get<double>());

    worker->Call("mitk_worker_test.release_kept");
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestProgressAndCancel()
  {
    auto worker = StartWorker();

    int reports = 0;

    const auto result = worker->Call("mitk_worker_test.count", { { "total", 1000 }, { "delay", 0.01 } }, {},
      [&](const nlohmann::json& progress) {
        if (progress.is_null())
          return true;

        CPPUNIT_ASSERT_EQUAL(1000, progress["total"].get<int>());
        return ++reports < 20;
      });

    // Ended by the cancel, not completed. How soon depends on the load of the
    // machine, so it is not asserted.
    CPPUNIT_ASSERT_MESSAGE("The call should end because of the cancel", result.contains("cancelled_at"));
    CPPUNIT_ASSERT(result["cancelled_at"].get<int>() < 1000);

    int ticks = 0;

    const auto completed = worker->Call("mitk_worker_test.count", { { "total", 2 }, { "delay", 0.5 } }, {},
      [&](const nlohmann::json& progress) {
        if (progress.is_null())
          ++ticks;

        return true;
      });

    CPPUNIT_ASSERT_EQUAL(2, completed["completed"].get<int>());
    CPPUNIT_ASSERT_MESSAGE("The hook should be called between sparse reports", ticks >= 1);

    // A call from inside the hook, e.g. from an event it processes, is refused
    // before anything is sent, and the outer call completes as usual.
    bool nestedTried = false;
    bool nestedRefused = false;

    const auto outer = worker->Call("mitk_worker_test.count", { { "total", 3 }, { "delay", 0.05 } }, {},
      [&](const nlohmann::json&) {
        if (!nestedTried)
        {
          nestedTried = true;

          try
          {
            worker->Call("mitk_worker_test.echo");
          }
          catch (const mitk::PythonWorkerError&)
          {
          }
          catch (const mitk::PythonWorkerExitedException&)
          {
          }
          catch (const mitk::PythonWorkerException&)
          {
            nestedRefused = true;
          }
        }

        return true;
      });

    CPPUNIT_ASSERT_MESSAGE("A nested call should be refused by MITK", nestedRefused);
    CPPUNIT_ASSERT_EQUAL(3, outer["completed"].get<int>());

    // A function that ignores the cancellation still delivers its result.
    const auto ignored = worker->Call("mitk_worker_test.ignore_cancel", { { "steps", 5 } }, {},
      [](const nlohmann::json&) { return false; });

    CPPUNIT_ASSERT_EQUAL(nlohmann::json("done"), ignored);
  }

  void TestCallsFromAnotherThread()
  {
    auto worker = StartWorker();

    // Tools make their calls off the GUI thread that started the worker.
    nlohmann::json result;
    std::thread([&] { result = worker->Call("mitk_worker_test.echo", { { "text", "off" } }); }).join();
    CPPUNIT_ASSERT_EQUAL(nlohmann::json({ { "text", "off" } }), result);

    // Calls take turns: one that starts while another one runs is refused.
    std::atomic<bool> started = false;
    std::atomic<bool> attempted = false;
    bool longCallFailed = false;

    std::thread longCall([&] {
      try
      {
        worker->Call("mitk_worker_test.count", { { "total", 10 }, { "delay", 0.05 } }, {},
          [&](const nlohmann::json&) {
            started = true;

            // Holds the call until the overlapping one was tried, however long a
            // loaded machine takes to get there.
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);

            while (!attempted && std::chrono::steady_clock::now() < deadline)
              std::this_thread::sleep_for(std::chrono::milliseconds(5));

            return true;
          });
      }
      catch (...)
      {
        longCallFailed = true;
      }
    });

    while (!started)
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

    bool refused = false;

    try
    {
      worker->Call("mitk_worker_test.echo");
    }
    catch (const mitk::PythonWorkerError&)
    {
    }
    catch (const mitk::PythonWorkerExitedException&)
    {
    }
    catch (const mitk::PythonWorkerException&)
    {
      refused = true;
    }

    attempted = true;
    longCall.join();

    CPPUNIT_ASSERT_MESSAGE("An overlapping call should be refused", refused);
    CPPUNIT_ASSERT_MESSAGE("The running call should complete", !longCallFailed);
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), worker->Call("mitk_worker_test.echo"));
  }

  void TestThrowingHookEndsWorker()
  {
    auto worker = StartWorker();

    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.count", { { "total", 1000 }, { "delay", 0.01 } }, {},
      [](const nlohmann::json&) -> bool { throw std::runtime_error("hook failed"); }), std::runtime_error);

    CPPUNIT_ASSERT(WaitUntilStopped(*worker));
    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.echo"), mitk::PythonWorkerExitedException);
  }

  void TestExit()
  {
    auto worker = StartWorker();
    CPPUNIT_ASSERT(worker->GetExitDescription().empty());

    try
    {
      worker->Call("mitk_worker_test.exit", { { "code", 3 } });
      CPPUNIT_FAIL("The exit should be reported");
    }
    catch (const mitk::PythonWorkerExitedException& e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.GetDescription(), std::string(e.GetDescription()).find("exited with code 3") != std::string::npos);
    }

    CPPUNIT_ASSERT(!worker->IsRunning());
    CPPUNIT_ASSERT(worker->GetExitDescription().find("exited with code 3") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("A worker that ended does not use its environment any more",
      !mitk::PythonWorker::IsAnyRunning(mitk::PythonHelper::GetExecutablePath()));
    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.echo"), mitk::PythonWorkerExitedException);
  }

  void TestCrash()
  {
    auto worker = StartWorker();

    try
    {
      worker->Call("mitk_worker_test.crash");
      CPPUNIT_FAIL("The crash should be reported");
    }
    catch (const mitk::PythonWorkerExitedException& e)
    {
      // The stack dump of faulthandler names the cause.
      const std::string description = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE(description, description.find("Segmentation fault") != std::string::npos);
      CPPUNIT_ASSERT_MESSAGE(description, description.find("in crash") != std::string::npos);
    }
  }

  void TestWorkersAreIndependent()
  {
    auto first = StartWorker();
    auto second = StartWorker();

    CPPUNIT_ASSERT_THROW(first->Call("mitk_worker_test.exit", { { "code", 1 } }), mitk::PythonWorkerExitedException);
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::object(), second->Call("mitk_worker_test.echo"));
    CPPUNIT_ASSERT_MESSAGE("The second worker still uses the environment",
      mitk::PythonWorker::IsAnyRunning(mitk::PythonHelper::GetExecutablePath()));
  }

  void TestAccessViolation()
  {
    auto worker = StartWorker();
    const auto start = std::chrono::steady_clock::now();

    try
    {
      worker->Call("mitk_worker_test.access_violation");
      CPPUNIT_FAIL("The crash should be reported");
    }
    catch (const mitk::PythonWorkerExitedException& e)
    {
      const std::string description = e.GetDescription();

#if defined(_WIN32)
      CPPUNIT_ASSERT_MESSAGE(description, description.find("access violation") != std::string::npos);
#else
      CPPUNIT_ASSERT_MESSAGE(description, description.find("Segmentation fault") != std::string::npos);
#endif
      CPPUNIT_ASSERT_MESSAGE(description, description.find("in access_violation") != std::string::npos);
    }

#if defined(_WIN32)
    // No crash dialog keeps the process alive. Elsewhere, crash handlers like
    // apport take their time but always finish, so a budget would only
    // measure their speed.
    CPPUNIT_ASSERT(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
#else
    static_cast<void>(start);
#endif
  }

  void TestForkedChildDoesNotHideExit()
  {
#if !defined(_WIN32)
    auto worker = StartWorker();

    // A child that keeps the channels open would let the call wait for it.
    const auto start = std::chrono::steady_clock::now();
    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.fork_and_exit"), mitk::PythonWorkerExitedException);
    CPPUNIT_ASSERT(std::chrono::steady_clock::now() - start < std::chrono::seconds(10));
#endif
  }

  void TestOrphanExits()
  {
    // A worker whose input closes, e.g. because MITK crashed, ends itself
    // instead of holding its memory and its environment.
    auto worker = StartWorker();
    const auto exitCode = worker->Call("mitk_worker_test.orphan_exits", { { "runtime_path", MITK_PYTHON_WORKER_RUNTIME } });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A worker should exit when its input closes", nlohmann::json(0), exitCode);
  }

  void TestDestroyingBusyWorkerDoesNotWait()
  {
    auto worker = StartWorker();

    // A worker that holds the GIL cannot even react to the end of its input,
    // so only killing it ends it quickly.
    worker->Call("mitk_worker_test.hog_interpreter_later");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const auto start = std::chrono::steady_clock::now();
    worker.reset();
    CPPUNIT_ASSERT_MESSAGE("Destroying a worker should not wait for it",
      std::chrono::steady_clock::now() - start < std::chrono::seconds(3));
  }

  void TestDescendantsEndWithWorker()
  {
    std::vector<std::filesystem::path> pythons = { mitk::PythonHelper::GetExecutablePath() };

#if defined(_WIN32)
    // The python.exe of a virtual environment is a launcher that puts the
    // interpreter into a job of its own, which lets the children of the
    // interpreter break away from the jobs above.
    struct TemporaryDirectory
    {
      ~TemporaryDirectory()
      {
        std::error_code error;
        std::filesystem::remove_all(Path, error);
      }

      std::filesystem::path Path;
    } venv{ std::filesystem::temp_directory_path() / ("mitkPythonWorkerTest_" + std::to_string(GetCurrentProcessId())) };

    StartWorker()->Call("mitk_worker_test.create_venv", { { "path", ToUtf8(venv.Path) } });
    pythons.push_back(venv.Path / "Scripts" / "python.exe");
#endif

    for (const auto& python : pythons)
    {
      auto worker = std::make_unique<mitk::PythonWorker>(python,
        std::vector<mitk::PythonWorker::Module>{ { "mitk_worker_test", ReadTestModule() } }, "test worker");

      const auto pid = worker->Call("mitk_worker_test.spawn_sleeper").get<int>();
      worker.reset();

      CPPUNIT_ASSERT_MESSAGE("A child of the worker should end with it: " + ToUtf8(python), WaitUntilGone(pid));
    }
  }

  void TestDescendantsEndWithExitedWorker()
  {
    auto worker = StartWorker();
    const auto pid = worker->Call("mitk_worker_test.spawn_sleeper").get<int>();

    CPPUNIT_ASSERT_THROW(worker->Call("mitk_worker_test.exit", { { "code", 5 } }), mitk::PythonWorkerExitedException);
    CPPUNIT_ASSERT_MESSAGE("A child of the worker should end once the worker ended", WaitUntilGone(pid));
    CPPUNIT_ASSERT_MESSAGE(worker->GetExitDescription(), worker->GetExitDescription().find("exited with code 5") != std::string::npos);
  }

  void TestDescendantsEndWithMitk()
  {
    auto worker = StartWorker();

    const auto pids = worker->Call("mitk_worker_test.run_and_kill_host", {
      { "host", MITK_PYTHON_WORKER_TEST_HOST },
      { "python", ToUtf8(mitk::PythonHelper::GetExecutablePath()) },
      { "module", MITK_PYTHON_WORKER_TEST_MODULE }
    });

    CPPUNIT_ASSERT_MESSAGE("The worker of a crashed MITK should end", WaitUntilGone(pids["worker"].get<int>()));
    CPPUNIT_ASSERT_MESSAGE("The child of that worker should end as well", WaitUntilGone(pids["child"].get<int>()));
  }

  void TestNoHandleLeaks()
  {
    // The first worker may set up what all of them share.
    StartWorker()->Call("mitk_worker_test.echo");
    const auto before = CountOpenHandles();

    for (int i = 0; i < 10; ++i)
      StartWorker()->Call("mitk_worker_test.echo");

    const auto after = CountOpenHandles();

    CPPUNIT_ASSERT_MESSAGE("Ten workers should not leave handles behind: " + std::to_string(before) + " before, " + std::to_string(after) + " after",
      after <= before + 5);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPythonWorker)
