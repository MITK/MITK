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

  class ExternalProcessOutputEvent : public itk::AnyEvent
  {
  public:
    typedef ExternalProcessOutputEvent Self;
    typedef itk::AnyEvent Superclass;

    explicit ExternalProcessOutputEvent(const std::string &output = "") : m_Output(output) {}
    ~ExternalProcessOutputEvent() override {}

    const char *GetEventName() const override { return "ExternalProcessOutputEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    itk::EventObject *MakeObject() const override { return new Self(m_Output); }
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
    virtual const char *GetEventName() const { return #classname; }                                                    \
    virtual bool CheckEvent(const ::itk::EventObject *e) const { return dynamic_cast<const Self *>(e); }               \
    virtual ::itk::EventObject *MakeObject() const { return new Self(this->GetOutput()); }                             \
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

    using ArgumentListType = std::vector<std::string>;

    bool Execute(const std::string &executionPath, const std::string &executableName, ArgumentListType &argumentList);

    /**
     * @brief Executes the process. This version assumes that the executable name is the first argument in the argument
     * list and has already been converted to its OS dependent name via the static convert function of this class.
     */
    bool Execute(const std::string &executionPath, const ArgumentListType &argumentList);

    int GetExitValue();
    static std::string EnsureCorrectOSPathSeparator(const std::string &);

    static std::string GetOSDependendExecutableName(const std::string &name);

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
