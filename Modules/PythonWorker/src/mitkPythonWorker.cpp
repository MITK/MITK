/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Asio has to see winsock2.h before anything includes windows.h.
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/asio/write.hpp>
#include <boost/process/v2/process.hpp>
#include <boost/process/v2/stdio.hpp>

#if defined(_WIN32)
#include <boost/process/v2/windows/creation_flags.hpp>
#else
#include <cerrno>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <mitkPythonWorker.h>
#include <mitkSharedBuffer.h>

#include <mitkExceptionMacro.h>
#include <mitkLog.h>
#include <mitkMemoryUtilities.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <thread>
#include <type_traits>
#include <utility>

namespace asio = boost::asio;
namespace bp = boost::process::v2;

using namespace std::chrono_literals;

namespace
{
  // Reads the first line of stdin, the runtime and the modules as JSON, and runs
  // the runtime on the rest of the input. It reads through a duplicate of the
  // descriptor, so later requests stay in the buffer of the same reader. Free of
  // spaces and double quotes, so it needs no quoting on any platform.
  const std::string BOOTSTRAP =
    "import json,os;"
    "i=os.fdopen(os.dup(0),'rb');"
    "m=json.loads(i.readline());"
    "exec(compile(m['runtime'],'mitk_worker.py','exec'),{'__name__':'__main__','mitk_input':i,'mitk_modules':m['modules'],"
    "'mitk_runtime':m['runtime'],'mitk_max_message_size':m['max_message_size']})";

  constexpr std::size_t ERROR_TAIL_LINES = 20;

  // Large data goes through SharedBuffer. A longer message comes from a broken
  // worker, which must not make MITK allocate without bound. The runtime
  // refuses to send one.
  constexpr std::size_t MAX_MESSAGE_SIZE = 16 * 1024 * 1024;

  // Progress bars redraw their line with carriage returns only, so a line of
  // stderr can grow for as long as they run.
  constexpr std::size_t MAX_ERROR_LINE_SIZE = 64 * 1024;

  std::string ReadRuntime()
  {
    auto module = us::GetModuleContext()->GetModule();
    auto resource = module->GetResource("mitk_worker.py");

    if (!resource.IsValid())
      mitkThrowException(mitk::PythonWorkerException) << "The resource mitk_worker.py could not be found.";

    us::ModuleResourceStream stream(resource);

    std::ostringstream source;
    source << stream.rdbuf();

    return source.str();
  }

  void IgnoreBrokenPipes()
  {
#if !defined(_WIN32)
    // Writing to a worker that is gone must fail with an error instead of
    // ending MITK. Leave a handler that someone else installed alone.
    static std::once_flag once;

    std::call_once(once, [] {
      struct sigaction current = {};

      if (sigaction(SIGPIPE, nullptr, &current) == 0 && current.sa_handler == SIG_DFL)
        std::signal(SIGPIPE, SIG_IGN);
    });
#endif
  }

  std::string DescribeExit(bp::native_exit_code_type status)
  {
    std::ostringstream description;

#if defined(_WIN32)
    if (status >= 0x80000000u)
      description << "exited with code 0x" << std::hex << std::uppercase << status;
    else
      description << "exited with code " << status;
#else
    if (WIFSIGNALED(status))
      description << "was terminated by signal " << WTERMSIG(status);
    else if (WIFEXITED(status))
      description << "exited with code " << WEXITSTATUS(status);
    else
      description << "exited";
#endif

    return description.str();
  }

#if defined(_WIN32)
  struct HandleCloser
  {
    void operator()(HANDLE handle) const
    {
      CloseHandle(handle);
    }
  };

  using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleCloser>;

  // The exit code that Boost.Process gives a process it terminates.
  constexpr UINT TERMINATED_EXIT_CODE = 260;

  // A job that ends every process in it once its last handle is closed, which
  // the system does when MITK crashes. No process can break away from it.
  UniqueHandle CreateKillOnCloseJob()
  {
    UniqueHandle job(CreateJobObjectW(nullptr, nullptr));

    if (job == nullptr)
      mitkThrowException(mitk::PythonWorkerException) << "Could not create a job object for a Python worker: error " << GetLastError();

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;

    if (!SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
      mitkThrowException(mitk::PythonWorkerException) << "Could not set up the job object of a Python worker: error " << GetLastError();

    return job;
  }

  // Puts the worker into its job before it runs. The python.exe of a virtual
  // environment is a launcher that starts the interpreter at once, in a job
  // of its own that lets the children of the interpreter break away. In a job
  // that forbids that, they stay.
  struct JobAssignment
  {
    HANDLE Job;
    DWORD Error = 0;

    template <typename Launcher, typename Path>
    boost::system::error_code on_setup(Launcher& launcher, const Path&, std::wstring&)
    {
      launcher.creation_flags |= CREATE_SUSPENDED;
      return {};
    }

    // Called before Boost.Process closes the handle of the main thread.
    template <typename Launcher, typename Path>
    void on_success(Launcher& launcher, const Path&, std::wstring&)
    {
      const auto& process = launcher.process_information;

      if (!AssignProcessToJobObject(this->Job, process.hProcess))
      {
        this->Error = GetLastError();
        TerminateProcess(process.hProcess, TERMINATED_EXIT_CODE);
      }

      ResumeThread(process.hThread);
    }
  };
#else
  // Makes the worker the leader of a process group, which the processes it
  // starts join, so that killing the group ends all of them.
  struct OwnProcessGroup
  {
    template <typename Launcher, typename Path>
    boost::system::error_code on_exec_setup(Launcher&, const Path&, const char* const*&)
    {
      if (setpgid(0, 0) == -1)
        return { errno, boost::system::system_category() };

      return {};
    }
  };
#endif

  std::mutex RegistryMutex;
  std::set<const void*> Registry;
}

struct mitk::PythonWorker::Impl
{
  Impl(const std::filesystem::path& python, const std::string& logPrefix)
    : Python(python),
      LogPrefix(logPrefix),
      Input(Context),
      Output(Context),
      Errors(Context),
      ExitTimer(Context)
  {
  }

  ~Impl()
  {
    {
      std::lock_guard lock(RegistryMutex);
      Registry.erase(this);
    }

    this->Context.stop();

    if (this->IOThread.joinable())
      this->IOThread.join();

    this->Kill();
  }

  void ReadOutput()
  {
    asio::async_read_until(this->Output, asio::dynamic_buffer(this->OutputData, MAX_MESSAGE_SIZE), '\n',
      [this](const boost::system::error_code& error, std::size_t length) {
        if (error == asio::error::not_found)
        {
          // The worker is broken. Its end is reported like any other.
          this->MessageTooLarge = true;
          this->OutputData = std::string();
          this->Kill();
          this->OnOutputClosed();
          return;
        }

        if (error)
        {
          this->OnOutputClosed();
          return;
        }

        const auto line = this->OutputData.substr(0, length - 1);
        this->OutputData.erase(0, length);
        this->OnMessage(line);
        this->ReadOutput();
      });
  }

  void ReadErrors()
  {
    asio::async_read_until(this->Errors, asio::dynamic_buffer(this->ErrorData, MAX_ERROR_LINE_SIZE), '\n',
      [this](const boost::system::error_code& error, std::size_t length) {
        if (error == asio::error::not_found)
        {
          this->OnErrorLine(std::exchange(this->ErrorData, std::string()));
          this->ReadErrors();
          return;
        }

        if (error)
        {
          // The last words of a crash may lack the final line break.
          if (!this->ErrorData.empty())
            this->OnErrorLine(std::exchange(this->ErrorData, std::string()));

          this->ErrorsClosed = true;

          if (this->OutputClosed)
            this->Finish();

          return;
        }

        auto line = this->ErrorData.substr(0, length - 1);
        this->ErrorData.erase(0, length);
        this->OnErrorLine(line);
        this->ReadErrors();
      });
  }

  void OnMessage(const std::string& line)
  {
    auto message = nlohmann::json::parse(line, nullptr, false);
    const bool isObject = !message.is_discarded() && message.is_object();

    // Python's startup, e.g. the .pth files of the venv, may write to this
    // channel before the runtime redirects stdout and reports ready.
    if (!this->RuntimeReady)
    {
      if (isObject && message.contains("ready"))
        this->RuntimeReady = true;
      else
        MITK_WARN << this->LogPrefix << ": " << line;

      return;
    }

    // From then on only the runtime writes to this channel, so a line that is
    // not a JSON object is a broken message of the current call, which must
    // not wait for a response that will never come.
    if (!isObject)
    {
      MITK_ERROR << this->LogPrefix << ": unreadable message: " << line;
      message = { { "unreadable", line } };
    }

    {
      std::lock_guard lock(this->Mutex);
      this->Messages.push_back(std::move(message));
    }

    this->Condition.notify_all();
  }

  void OnErrorLine(std::string line)
  {
    // Progress bars redraw a line with carriage returns; only the final state
    // is worth logging.
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    if (const auto lastReturn = line.rfind('\r'); lastReturn != std::string::npos)
      line.erase(0, lastReturn + 1);

    if (line.empty())
      return;

    MITK_INFO << this->LogPrefix << ": " << line;

    std::lock_guard lock(this->Mutex);
    this->ErrorTail.push_back(line);

    if (this->ErrorTail.size() > ERROR_TAIL_LINES)
      this->ErrorTail.pop_front();
  }

  void OnOutputClosed()
  {
    this->OutputClosed = true;

    if (this->ErrorsClosed)
    {
      this->Finish();
      return;
    }

    // Gives stderr a moment to deliver the last words, e.g. a stack trace,
    // unless a child process of the worker keeps it open.
    this->ExitTimer.expires_after(1s);
    this->ExitTimer.async_wait([this](const boost::system::error_code&) { this->Finish(); });
  }

  // Runs on the I/O thread once the protocol channel is closed.
  void Finish()
  {
    if (this->Exited)
      return;

    this->ExitTimer.cancel();

    std::string exit = "exited";

    {
      std::lock_guard lock(this->ProcessMutex);

      if (this->Process.has_value())
      {
        if (!this->Reaped)
        {
          // A worker that ends on its own keeps its exit code.
          for (int i = 0; i < 50 && !this->HasEnded(); ++i)
            std::this_thread::sleep_for(20ms);

          // What the worker started is of no use without it.
          this->EndProcessTree();

          boost::system::error_code error;

          if (this->Process->running(error))
            this->Process->terminate(error);

          this->ReleaseProcess();
        }

        exit = DescribeExit(this->Process->native_exit_code());
      }
    }

    if (this->MessageTooLarge)
      exit = "sent a message larger than " + MemoryUtilities::FormatSize(MAX_MESSAGE_SIZE) + " and was ended";

    {
      std::lock_guard lock(this->Mutex);

      std::ostringstream description;
      description << "The Python worker " << this->LogPrefix << ' ' << exit << '.';

      if (!this->ErrorTail.empty())
      {
        description << "\n\nIts last output:";

        for (const auto& line : this->ErrorTail)
          description << '\n' << line;
      }

      this->ExitDescription = description.str();
      this->Exited = true;
    }

    this->Condition.notify_all();
  }

  [[noreturn]] void ThrowExited() const
  {
    std::string description;

    {
      std::lock_guard lock(this->Mutex);
      description = this->ExitDescription;
    }

    if (description.empty())
      description = "The Python worker " + this->LogPrefix + " is not running.";

    mitkThrowException(PythonWorkerExitedException) << description;
  }

  void Send(const nlohmann::json& message)
  {
    const auto line = message.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace) + '\n';

    boost::system::error_code error;
    asio::write(this->Input, asio::buffer(line), error);

    if (error)
    {
      // The worker is gone; wait for the I/O thread to find out why.
      this->WaitForExit();
      this->ThrowExited();
    }
  }

  void WaitForExit()
  {
    std::unique_lock lock(this->Mutex);
    this->Condition.wait_for(lock, 3s, [this] { return this->Exited.load(); });
  }

  void Kill()
  {
    std::lock_guard lock(this->ProcessMutex);

    if (this->Process.has_value() && !this->Reaped)
    {
      this->EndProcessTree();

      boost::system::error_code error;

      // running() stores the exit code of a process that already ended, which
      // terminate() would otherwise leave at STILL_ACTIVE on Windows. If the
      // query fails, the process may still run.
      if (this->Process->running(error) || error)
        this->Process->terminate(error);

      this->ReleaseProcess();
    }
  }

  // Once reaped, the process ID can belong to another process, which the
  // destructor of bp::process would kill nonetheless. The exit code survives
  // detaching.
  void ReleaseProcess()
  {
    this->Process->detach();
    this->Reaped = true;
  }

  // Unlike bp::process::running(), this does not reap the worker.
  bool HasEnded()
  {
#if defined(_WIN32)
    return WaitForSingleObject(this->Process->native_handle(), 0) == WAIT_OBJECT_0;
#else
    // An error means that there is nothing to wait for.
    siginfo_t info = {};
    const auto pid = this->Process->id();
    return waitid(P_PID, pid, &info, WEXITED | WNOHANG | WNOWAIT) == -1 || info.si_pid == pid;
#endif
  }

  // Ends the worker and every process it started. Only while the worker is not
  // reaped: on POSIX, its process ID could name another process group then.
  void EndProcessTree()
  {
#if defined(_WIN32)
    TerminateJobObject(this->Job.get(), TERMINATED_EXIT_CODE);
#else
    killpg(this->Process->id(), SIGKILL);
#endif
  }

  const std::filesystem::path Python;
  const std::string LogPrefix;

#if defined(_WIN32)
  UniqueHandle Job;
#endif

  asio::io_context Context;
  asio::writable_pipe Input;
  asio::readable_pipe Output;
  asio::readable_pipe Errors;
  asio::steady_timer ExitTimer;
  std::thread IOThread;

  // Killed from the calling thread, reaped on the I/O thread.
  std::mutex ProcessMutex;
  std::optional<bp::process> Process;
  bool Reaped = false;

  std::string OutputData;
  std::string ErrorData;
  bool MessageTooLarge = false;
  bool OutputClosed = false;
  bool ErrorsClosed = false;
  bool RuntimeReady = false;

  mutable std::mutex Mutex;
  std::condition_variable Condition;
  std::deque<nlohmann::json> Messages;
  std::deque<std::string> ErrorTail;
  std::string ExitDescription;
  std::atomic<bool> Exited = false;

  std::int64_t NextId = 1;
  bool InCall = false;
};

mitk::PythonWorker::PythonWorker(const std::filesystem::path& python, const std::vector<Module>& modules, const std::string& logPrefix)
  : m_Impl(std::make_unique<Impl>(python, logPrefix))
{
  IgnoreBrokenPipes();

  nlohmann::json startup = {
    { "runtime", ReadRuntime() },
    { "modules", nlohmann::json::array() },
    { "max_message_size", MAX_MESSAGE_SIZE }
  };

  for (const auto& module : modules)
    startup["modules"].push_back({ { "name", module.Name }, { "source", module.Source } });

#if defined(_WIN32)
  m_Impl->Job = CreateKillOnCloseJob();
  JobAssignment jobAssignment{ m_Impl->Job.get() };
#else
  OwnProcessGroup ownProcessGroup;
#endif

  try
  {
    m_Impl->Process.emplace(m_Impl->Context, python, std::vector<std::string>{ "-I", "-c", BOOTSTRAP },
      bp::process_stdio{ m_Impl->Input, m_Impl->Output, m_Impl->Errors }
#if defined(_WIN32)
      , bp::windows::process_creation_flags<CREATE_NO_WINDOW>()
      , jobAssignment
#else
      , ownProcessGroup
#endif
    );
  }
  catch (const std::exception& e)
  {
    mitkThrowException(PythonWorkerException) << "Could not start " << python.string() << ": " << e.what();
  }

#if defined(_WIN32)
  if (jobAssignment.Error != 0)
    mitkThrowException(PythonWorkerException) << "Could not start " << python.string() << " in a job object: error " << jobAssignment.Error;
#endif

  m_Impl->ReadOutput();
  m_Impl->ReadErrors();
  m_Impl->IOThread = std::thread([this] { m_Impl->Context.run(); });

  {
    std::lock_guard lock(RegistryMutex);
    Registry.insert(m_Impl.get());
  }

  m_Impl->Send(startup);
}

mitk::PythonWorker::~PythonWorker() = default;

nlohmann::json mitk::PythonWorker::Call(const std::string& method, const nlohmann::json& params, const Buffers& buffers, const ProgressHook& progress)
{
  auto& impl = *m_Impl;

  if (impl.InCall)
    mitkThrowException(PythonWorkerException) << "Calls of a Python worker must not be nested.";

  if (!params.is_object() && !params.is_null())
    mitkThrowException(PythonWorkerException) << "The parameters of " << method << " must be a JSON object.";

  if (impl.Exited)
    impl.ThrowExited();

  const auto id = impl.NextId++;

  nlohmann::json request = {
    { "id", id },
    { "method", method },
    { "params", params.is_null() ? nlohmann::json::object() : params },
    { "buffers", nlohmann::json::object() }
  };

  for (const auto& [key, buffer] : buffers)
  {
    if (buffer == nullptr)
      mitkThrowException(PythonWorkerException) << "The buffer " << key << " of " << method << " is null.";

    if (!buffer->IsPublished())
      mitkThrowException(PythonWorkerException) << "The buffer " << key << " of " << method << " was passed to an earlier call.";

    request["buffers"][key] = buffer->GetDescriptor();
  }

  struct CallScope
  {
    CallScope(Impl& impl, const Buffers& buffers) : m_Impl(impl), m_Buffers(buffers) { m_Impl.InCall = true; }

    ~CallScope()
    {
      // Nothing may attach any more, so a crash cannot leave a buffer behind.
      for (const auto& [key, buffer] : m_Buffers)
        buffer->Unpublish();

      m_Impl.InCall = false;
    }

    Impl& m_Impl;
    const Buffers& m_Buffers;
  } scope(impl, buffers);

  impl.Send(request);

  bool cancelRequested = false;

  auto callHook = [&](const nlohmann::json& fields) {
    if (!progress)
      return;

    bool proceed = true;

    try
    {
      proceed = progress(fields);
    }
    catch (...)
    {
      // The response of the call would otherwise be taken for the response of
      // the next one.
      impl.Kill();
      throw;
    }

    if (!proceed && !cancelRequested)
    {
      cancelRequested = true;
      impl.Send({ { "cancel", id } });
    }
  };

  while (true)
  {
    std::unique_lock lock(impl.Mutex);
    const auto ready = [&impl] { return !impl.Messages.empty() || impl.Exited; };

    if (progress)
      impl.Condition.wait_for(lock, 100ms, ready);
    else
      impl.Condition.wait(lock, ready);

    if (impl.Messages.empty())
    {
      lock.unlock();

      if (impl.Exited)
        impl.ThrowExited();

      callHook(nlohmann::json());
      continue;
    }

    // Reports that arrive faster than the hook handles them would pile up in
    // front of the result. Of consecutive reports, only the newest counts.
    const auto isProgress = [id](const nlohmann::json& message) {
      const auto messageId = message.find("id");
      return messageId != message.end() && *messageId == id && message.contains("progress");
    };

    while (impl.Messages.size() > 1 && isProgress(impl.Messages[0]) && isProgress(impl.Messages[1]))
      impl.Messages.pop_front();

    auto message = std::move(impl.Messages.front());
    impl.Messages.pop_front();
    lock.unlock();

    if (const auto unreadable = message.find("unreadable"); unreadable != message.end() && !message.contains("id"))
    {
      // The worker is in an unknown state and may still be busy with this
      // call. Waiting for the I/O thread lets IsRunning() report the end, so
      // tools treat the worker as lost.
      impl.Kill();
      impl.WaitForExit();

      constexpr std::size_t maxQuoted = 200;
      const auto line = unreadable->get<std::string>();

      mitkThrowException(PythonWorkerExitedException) << "The Python worker " << impl.LogPrefix
        << " sent a message that is not valid JSON and was ended: " << line.substr(0, maxQuoted)
        << (line.size() > maxQuoted ? "..." : "");
    }

    if (const auto messageId = message.find("id"); messageId == message.end() || *messageId != id)
      continue;

    if (auto fields = message.find("progress"); fields != message.end())
    {
      callHook(*fields);
      continue;
    }

    if (auto result = message.find("result"); result != message.end())
      return *result;

    const auto error = message.value("error", nlohmann::json::object());
    const auto type = error.value("type", std::string("Exception"));
    const auto pythonMessage = error.value("message", std::string());
    const auto description = pythonMessage.empty() ? type : type + ": " + pythonMessage;

    PythonWorkerError exception(__FILE__, __LINE__, description.c_str(), ITK_LOCATION);
    exception.SetPythonType(type);
    exception.SetTraceback(error.value("traceback", std::string()));

    throw exception;
  }
}

bool mitk::PythonWorker::IsRunning() const
{
  return !m_Impl->Exited;
}

std::string mitk::PythonWorker::GetExitDescription() const
{
  std::lock_guard lock(m_Impl->Mutex);
  return m_Impl->ExitDescription;
}

bool mitk::PythonWorker::IsAnyRunning(const std::filesystem::path& python)
{
  std::lock_guard lock(RegistryMutex);

  for (const auto* entry : Registry)
  {
    const auto* impl = static_cast<const Impl*>(entry);

    if (!impl->Exited && impl->Python == python)
      return true;
  }

  return false;
}
