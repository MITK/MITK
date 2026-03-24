/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentAnythingProcessExecutor_h
#define mitkSegmentAnythingProcessExecutor_h

#include <MitkSegmentationExports.h>
#include <mitkProcessExecutor.h>
#include <itksys/Process.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * @brief You may register an observer for an ExternalProcessOutputEvent, ExternalProcessStdOutEvent or
   * ExternalProcessStdErrEvent in order to get notified of any output.
   * @remark The events will only be invoked if the pipes are NOT(!) shared. By default the pipes are not shared.
   *
   */
  class MITKSEGMENTATION_EXPORT SegmentAnythingProcessExecutor : public mitk::ProcessExecutor
  {
  public:
    using Self = SegmentAnythingProcessExecutor;
    using Superclass = mitk::ProcessExecutor;
    using Pointer = ::itk::SmartPointer<Self>;
    using ConstPointer = ::itk::SmartPointer<const Self>;
    using mitk::ProcessExecutor::Execute;

    itkTypeMacro(SegmentAnythingProcessExecutor, mitk::ProcessExecutor);
    mitkNewMacro1Param(SegmentAnythingProcessExecutor, double&);

    itkSetMacro(Stop, bool);
    itkGetConstMacro(Stop, bool);

    /**
     * @brief Executes the process. This version assumes that the executable name is the first argument in the argument
     * list and has already been converted to its OS dependent name via the static convert function of this class.
     */
    bool Execute(const std::string &executionPath, const ArgumentListType &argumentList);

    void SetTimeout(double &timeout);

  protected:
    SegmentAnythingProcessExecutor() = delete;
    SegmentAnythingProcessExecutor(double &);
    ~SegmentAnythingProcessExecutor() = default;

  private:
    itksysProcess *m_ProcessID = nullptr;
    double m_Timeout;
    bool m_Stop = false; // Exit token to force stop the daemon.
  };

} // namespace mitk
#endif
