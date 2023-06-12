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
#include "mitkImage.h"


namespace mitk
{
  /** CHANGE THIS --ashis
  \brief Segment Anything Model interactive 2D tool.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT SegmentAnythingPythonService : public itk::Object
  {
  public: 
    enum Status
    {
      READY,
      PROCESSING,
      KILLED,
      CUDAError
    };

    SegmentAnythingPythonService(std::string, std::string, std::string, unsigned int);
    ~SegmentAnythingPythonService();
    
    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

    /**
     * @brief Static function to print out everything from itk::EventObject.
     * Used as callback in mitk::ProcessExecutor object.
     *
     */
    static void onPythonProcessEvent(itk::Object*, const itk::EventObject&, void*);
    bool static IsPythonReady;
    static Status CurrentStatus;
    static void CheckStatus();
    void StartAsyncProcess();
    void StopAsyncProcess();
    void TransferImageToProcess(const Image*, std::string &UId);
    //void TransferPointsToProcess(std::vector<std::pair<mitk::Point2D, std::string>>&);
    void TransferPointsToProcess(std::stringstream&);
    Image::Pointer RetrieveImageFromProcess();

  private:
    void start_python_daemon();
    void WriteControlFile(std::stringstream&);
    void CreateTempDirs(const std::string&);

    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_InDir, m_OutDir;
    std::string m_CurrentUId;
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
    static const std::string SUCCESS;
    static const std::string PROCESSING;
    static const std::string CUDA_OUT_OF_MEMORY_ERROR;
  };

} // namespace

#endif
