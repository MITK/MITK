/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProcessExecutor_h
#define mitkProcessExecutor_h

#include <MitkSegmentationExports.h>
#include <itkObject.h>
#include <vector>
#include <itksys/Process.h>

namespace mitk
{
  // Class is adapted from MatchPoint ProcessExecutor

  /**
   * \brief Event carrying output text from an external process.
   *
   * Base class for stdout/stderr events emitted during external process execution.
   * Register an observer for this event type to capture all process output.
   *
   * \sa ProcessExecutor, ExternalProcessStdOutEvent, ExternalProcessStdErrEvent
   */
  class ExternalProcessOutputEvent : public itk::AnyEvent
  {
  public:
    typedef ExternalProcessOutputEvent Self;
    typedef itk::AnyEvent Superclass;

    /**
     * \brief Constructor.
     * \param[in] output The output text from the external process.
     */
    explicit ExternalProcessOutputEvent(const std::string &output = "") : m_Output(output) {}
    ~ExternalProcessOutputEvent() override {}

    const char *GetEventName() const override { return "ExternalProcessOutputEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    itk::EventObject *MakeObject() const override { return new Self(m_Output); }

    /** \brief Returns the output text from the external process. */
    std::string GetOutput() const { return m_Output; }

  private:
    std::string m_Output;
  };

#define mitkProcessExecutorEventMacro(classname)                                                                       \
  class classname : public ExternalProcessOutputEvent                                                                  \
  {                                                                                                                    \
  public:                                                                                                              \
    typedef classname Self;                                                                                            \
    typedef ExternalProcessOutputEvent Superclass;                                                                     \
                                                                                                                       \
    explicit classname(const std::string &output) : Superclass(output) {}                                              \
    ~classname() override {}                                                                                           \
                                                                                                                       \
    virtual const char *GetEventName() const override { return #classname; }                                                    \
    virtual bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }               \
    virtual ::itk::EventObject *MakeObject() const override { return new Self(this->GetOutput()); }                             \
  };

  mitkProcessExecutorEventMacro(ExternalProcessStdOutEvent);
  mitkProcessExecutorEventMacro(ExternalProcessStdErrEvent);

  /**
   * @brief You may register an observer for an ExternalProcessOutputEvent, ExternalProcessStdOutEvent or
   * ExternalProcessStdErrEvent in order to get notified of any output.
   * @remark The events will only be invoked if the pipes are NOT(!) shared. By default the pipes are not shared.
   *
   */
  class MITKSEGMENTATION_EXPORT ProcessExecutor : public itk::Object
  {
  public:
    using Self = ProcessExecutor;
    using Superclass = ::itk::Object;
    using Pointer = ::itk::SmartPointer<Self>;
    using ConstPointer = ::itk::SmartPointer<const Self>;

    itkTypeMacro(ProcessExecutor, ::itk::Object);
    itkFactorylessNewMacro(Self);

    itkSetMacro(SharedOutputPipes, bool);
    itkGetConstMacro(SharedOutputPipes, bool);

    /** \brief Type for the list of command-line arguments. */
    using ArgumentListType = std::vector<std::string>;

    /**
     * \brief Executes an external process.
     * \param[in] executionPath Working directory for the process.
     * \param[in] executableName Name of the executable to run.
     * \param[in,out] argumentList Command-line arguments for the process.
     * \return true if the process executed successfully, false otherwise.
     */
    bool Execute(const std::string &executionPath, const std::string &executableName, ArgumentListType &argumentList);

    /**
     * \brief Executes an external process.
     *
     * This version assumes that the executable name is the first argument in the argument
     * list and has already been converted to its OS-dependent name.
     *
     * \param[in] executionPath Working directory for the process.
     * \param[in] argumentList Command-line arguments; first element is the executable.
     * \return true if the process executed successfully, false otherwise.
     */
    virtual bool Execute(const std::string &executionPath, const ArgumentListType &argumentList);

    /**
     * \brief Returns the exit value of the last executed process.
     * \return The process exit code.
     */
    int GetExitValue();

    /**
     * \brief Converts path separators to the OS-correct form.
     * \param[in] path The path string to convert.
     * \return The path with correct OS-specific separators.
     */
    static std::string EnsureCorrectOSPathSeparator(const std::string & path);

    /**
     * \brief Returns the OS-dependent executable name (e.g. adds ".exe" on Windows).
     * \param[in] name The platform-independent executable name.
     * \return The OS-dependent executable name.
     */
    static std::string GetOSDependendExecutableName(const std::string &name);

    /**
     * \brief Kills the currently running process.
     */
    void KillProcess();

  protected:
    ProcessExecutor();
    ~ProcessExecutor() override;

    int m_ExitValue;

    /**
     * @brief Specifies if the child process should share the output pipes (true) or not (false).
     * If pipes are not shared the output will be passed by invoking ExternalProcessOutputEvents
     * @remark The events will only be invoked if the pipes are NOT(!) shared.
     */
    bool m_SharedOutputPipes;

  private:
    itksysProcess *m_ProcessID = nullptr;
  };

} // namespace mitk
#endif
