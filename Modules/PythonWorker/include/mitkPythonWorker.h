/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonWorker_h
#define mitkPythonWorker_h

#include <mitkExceptionMacro.h>

#include <nlohmann/json.hpp>

#include <MitkPythonWorkerExports.h>

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mitk
{
  class SharedBuffer;

  /** \brief Base class of the exceptions thrown by PythonWorker. */
  class MITKPYTHONWORKER_EXPORT PythonWorkerException : public Exception
  {
  public:
    mitkExceptionClassMacro(PythonWorkerException, Exception);
  };

  /**
   * \brief The function a PythonWorker called raised a Python exception.
   *
   * The description is the Python exception as "Type: message", or the type
   * alone if the message is empty. The worker keeps running.
   */
  class MITKPYTHONWORKER_EXPORT PythonWorkerError : public PythonWorkerException
  {
  public:
    mitkExceptionClassMacro(PythonWorkerError, PythonWorkerException);

    /** \brief Name of the class of the Python exception, e.g. "ValueError". */
    const std::string& GetPythonType() const { return m_PythonType; }
    void SetPythonType(const std::string& pythonType) { m_PythonType = pythonType; }

    const std::string& GetTraceback() const { return m_Traceback; }
    void SetTraceback(const std::string& traceback) { m_Traceback = traceback; }

  private:
    std::string m_PythonType;
    std::string m_Traceback;
  };

  /**
   * \brief The process of a PythonWorker is gone, or was ended because a call could not complete.
   *
   * The description contains the exit code and the last lines the worker wrote
   * to stderr, which usually name the cause. Every further call throws this
   * exception again; create a new worker to continue.
   */
  class MITKPYTHONWORKER_EXPORT PythonWorkerExitedException : public PythonWorkerException
  {
  public:
    mitkExceptionClassMacro(PythonWorkerExitedException, PythonWorkerException);
  };

  /**
   * \brief A Python interpreter in a process of its own that serves function calls.
   *
   * Running Python code in a separate process keeps the packages of a virtual
   * environment out of MITK and out of other environments, and a crash of a
   * native library takes only the worker down.
   *
   * The worker runs the given modules and then calls their functions on request.
   * Arguments and results are JSON values. Large arrays are passed as
   * SharedBuffer objects, which the called function receives as NumPy arrays
   * on the same memory. A module can report progress with
   * `mitk_worker.report_progress(**fields)`, which returns False once MITK asked
   * to cancel the call.
   *
   * Everything the worker writes to stdout or stderr is logged with the given
   * prefix.
   *
   * The destructor kills the process right away, which also frees everything it
   * held, like GPU memory.
   *
   * All calls of a worker must come from the same thread.
   */
  class MITKPYTHONWORKER_EXPORT PythonWorker
  {
  public:
    struct Module
    {
      /** \brief Name under which other modules can import it. */
      std::string Name;
      std::string Source;
    };

    /**
     * \brief Called while a call is in progress.
     *
     * Receives the fields of each progress report, and null whenever 100 ms
     * pass without a report. Returning false asks the worker to cancel the call,
     * which is up to the called function. If the hook throws, the worker is
     * ended and the exception propagates.
     *
     * The hook must not destroy the worker, also not indirectly through events
     * it processes. Defer that until Call() has returned.
     */
    using ProgressHook = std::function<bool(const nlohmann::json& progress)>;

    using Buffers = std::map<std::string, SharedBuffer*>;

    /**
     * \brief Starts the interpreter and runs the modules in the given order.
     *
     * Errors in the modules are reported by the first call.
     */
    PythonWorker(const std::filesystem::path& python, const std::vector<Module>& modules, const std::string& logPrefix);
    ~PythonWorker();

    PythonWorker(const PythonWorker&) = delete;
    PythonWorker& operator=(const PythonWorker&) = delete;

    /**
     * \brief Calls a function of one of the modules and returns its result.
     *
     * \param method "module.function"
     * \param params Keyword arguments of the function, a JSON object or null.
     * \param buffers Further keyword arguments, received as NumPy arrays. Each
     *        buffer can be passed to one call only.
     * \param progress Optional hook for progress and cancellation. Without it,
     *        the call blocks the calling thread without processing events and
     *        cannot be interrupted, so long calls pass one.
     *
     * \throw PythonWorkerError if the function raised an exception.
     * \throw PythonWorkerExitedException if the worker is gone.
     */
    nlohmann::json Call(const std::string& method,
                        const nlohmann::json& params = nlohmann::json::object(),
                        const Buffers& buffers = {},
                        const ProgressHook& progress = {});

    /** \brief False once the process is known to be gone. */
    bool IsRunning() const;

    /**
     * \brief Why the process is gone: its exit code and the last lines it wrote to stderr.
     *
     * The same text as the description of PythonWorkerExitedException. Empty while
     * IsRunning() is true. The first line states the exit, the further lines
     * the last output.
     */
    std::string GetExitDescription() const;

    /** \brief Whether any worker of the given interpreter is running in this process. */
    static bool IsAnyRunning(const std::filesystem::path& python);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
