/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentAnythingPythonService_h
#define mitkSegmentAnythingPythonService_h

#include <mitkProcessExecutor.h>
#include <MitkSegmentationExports.h>
#include <thread>
#include <future>


namespace mitk
{
  /** CHANGE THIS --ashis
  \brief Segment Anything Model interactive 2D tool.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT SegmentAnythingPythonService 
  {
  public: 
    SegmentAnythingPythonService(std::string, std::string, std::string, std::string, std::string, unsigned int);
    ~SegmentAnythingPythonService();
    /**
     * @brief Static function to print out everything from itk::EventObject.
     * Used as callback in mitk::ProcessExecutor object.
     *
     */
    static void onPythonProcessEvent(itk::Object *, const itk::EventObject &e, void *);
    bool static IsPythonReady;
    void StartAsyncProcess();
    void StopAsyncProcess();

  private:
    void start_python_daemon();
    void WriteControlFile(std::stringstream&);

    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_InDir, m_OutDir;
    unsigned int m_GpuId = 0;
    bool m_IsAuto = false;
    bool m_IsReady = false;
    const std::string TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string m_PARENT_TEMP_DIR_PATTERN = "mitk-sam-XXXXXX";
    const std::string m_TRIGGER_FILENAME = "trigger.csv";
    std::future<void> m_Future;
    ProcessExecutor::Pointer m_DaemonExec;
  };


  struct SIGNALCONSTANTS
  {
    static const std::string READY;
    static const std::string KILL;
  };

} // namespace

#endif
