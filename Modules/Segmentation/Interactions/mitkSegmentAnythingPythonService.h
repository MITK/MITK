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

namespace mitk
{
  /**
   * @brief Segment Anything Model Python process handler class.
   * 
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

    /**
     * @brief Checks CurrentStatus enum variable and returns 
     * true if daemon is READY (to read files) state, false is OFF state or
     * throws exception if daemon is found KILL or Cuda error state.
     * 
     * @return bool 
     */
    static bool CheckStatus() /*throw(mitk::Exception)*/;

    /**
     * @brief Creates temp directories and calls start_python_daemon
     * function async.
     * 
     */
    void StartAsyncProcess();

    /**
     * @brief Writes KILL to the control file to stop the daemon process.
     * 
     */
    void StopAsyncProcess();

    /**
     * @brief Writes image as nifity file with unique id (UId) as file name. 
     * 
     */
    void TransferImageToProcess(const Image*, std::string &UId);

    /**
     * @brief Writes csv stringstream of points to a csv file for 
     * python daemon to read.
     * 
     */
    void TransferPointsToProcess(std::stringstream&);

    /**
     * @brief Waits for output nifity file from the daemon to appear and 
     * reads it as a mitk::Image
     * 
     * @return Image::Pointer 
     */
    LabelSetImage::Pointer RetrieveImageFromProcess(long timeOut= -1);

    static Status CurrentStatus;

  private:
    /**
     * @brief Runs SAM python daemon using mitk::ProcessExecutor
     * 
     */
    void start_python_daemon();

    /**
     * @brief Writes stringstream content into control file.
     * 
     */
    void WriteControlFile(std::stringstream&);

    /**
     * @brief Create a Temp Dirs
     * 
     */
    void CreateTempDirs(const std::string&);

    /**
     * @brief ITK-based file writer for dumping inputs into python daemon
     *
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWriter(const itk::Image<TPixel, VImageDimension> *image, std::string& outputFilename);


    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_InDir, m_OutDir;
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
