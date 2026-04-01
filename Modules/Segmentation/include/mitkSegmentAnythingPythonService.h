/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentAnythingPythonService_h
#define mitkSegmentAnythingPythonService_h

#include <mitkSegmentAnythingProcessExecutor.h>
#include <MitkSegmentationExports.h>
#include <thread>
#include <future>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <itkImage.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * \brief Manages the Python daemon process for the Segment Anything Model (SAM).
   *
   * Handles starting, stopping, and communicating with a Python daemon that runs
   * the SAM inference. Transfers images and point prompts via temporary files and
   * retrieves segmentation results.
   *
   * \sa SegmentAnythingTool, MedSAMTool
   */
  class MITKSEGMENTATION_EXPORT SegmentAnythingPythonService : public itk::Object
  {
  public: 
    enum Status
    {
      READY,
      OFF,
      KILLED,
      CUDAError
    };
    mitkClassMacroItkParent(SegmentAnythingPythonService, itk::Object);

    /**
     * \brief Constructs a new SegmentAnythingPythonService.
     *
     * \param[in] workingDir Working directory for the Python process.
     * \param[in] modelType The ViT model type identifier.
     * \param[in] checkPointPath Path to the model checkpoint file.
     * \param[in] gpuId GPU device ID to use for inference.
     * \param[in] backend Backend identifier: "SAM" or "MedSAM".
     */
    SegmentAnythingPythonService(std::string workingDir, std::string modelType,
                                 std::string checkPointPath, unsigned int gpuId, std::string backend);
    
    /**
     * \brief Destroys the service, stopping the async Python process and deleting temporary directories.
     */
    ~SegmentAnythingPythonService();
    
    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);
    mitkNewMacro5Param(SegmentAnythingPythonService, std::string, std::string, std::string, unsigned int, std::string);
    /**
     * \brief Static callback to print output from itk::EventObject events.
     *
     * Used as callback in mitk::ProcessExecutor.
     */
    static void onPythonProcessEvent(itk::Object*, const itk::EventObject&, void*);

    /**
     * \brief Checks the daemon status.
     *
     * \return true if the daemon is in READY state, false if OFF.
     * \throw mitk::Exception if the daemon is in KILLED or CUDAError state.
     */
    static bool CheckStatus() /*throw(mitk::Exception)*/;

    /**
     * \brief Creates temporary directories and starts the Python daemon asynchronously.
     */
    void StartAsyncProcess();

    /**
     * \brief Writes KILL signal to the control file to stop the daemon process.
     */
    void StopAsyncProcess();

    /**
     * \brief Writes an image as a NIfTI file with a unique ID as the file name.
     */
    void TransferImageToProcess(const Image *inputAtTimeStep, std::string &UId);

    /**
     * \brief Writes CSV point data to a trigger file for the Python daemon to read.
     */
    void TransferPointsToProcess(const std::string &triggerCSV) const;

    /**
     * \brief Waits for the output NIfTI file from the daemon and reads it as a segmentation.
     *
     * \param[in] timeOut Timeout in milliseconds (-1 for indefinite waiting).
     * \return The segmentation image produced by the daemon.
     */
    MultiLabelSegmentation::Pointer RetrieveImageFromProcess(long timeOut= -1) const;

    static Status CurrentStatus;

  private:
    /**
     * \brief Runs the SAM Python daemon using mitk::ProcessExecutor.
     */
    void start_python_daemon() const;

    /**
     * \brief Writes the given status string to the control file.
     */
    void WriteControlFile(const std::string &statusString) const;

    /**
     * \brief Creates temporary directories for I/O with the Python daemon.
     */
    void CreateTempDirs(const std::string &dirPattern);

    /**
     * \brief ITK-based file writer for writing input images to disk for the Python daemon.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWriter(const itk::Image<TPixel, VImageDimension> *image, std::string& outputFilename) const;


    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_InDir, m_OutDir;
    std::string m_Backend;
    std::string m_CurrentUId;
    int m_GpuId = 0;
    const std::string PARENT_TEMP_DIR_PATTERN = "mitk-sam-XXXXXX";
    const std::string TRIGGER_FILENAME = "trigger.csv";
    const std::string SAM_PYTHON_FILE_NAME = "run_inference_daemon.py";
    std::future<void> m_Future;
    SegmentAnythingProcessExecutor::Pointer m_DaemonExec;
  };

  struct SIGNALCONSTANTS
  {
    static const std::string READY;
    static const std::string KILL;
    static const std::string OFF;
    static const std::string CUDA_OUT_OF_MEMORY_ERROR;
    static const std::string TIMEOUT_ERROR;
  };

} // namespace

#endif
