/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnUnetTool_h
#define mitknnUnetTool_h

#include "mitkSegWithPreviewTool.h"
#include "mitkCommon.h"
#include "mitkToolManager.h"
#include <MitkSegmentationExports.h>
#include <mitkStandardFileLocations.h>
#include <numeric>
#include <utility>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * @brief nnUNet parameter request object holding all model parameters for input.
   * Also holds output temporary directory path.
   */
  struct ModelParams
  {
    std::string task;
    std::vector<std::string> folds;
    std::string model;
    std::string trainer;
    std::string planId;
    std::string outputDir;
    std::string inputName;
    std::string timeStamp;

    size_t generateHash() const
    {
      std::string toHash;
      std::string foldsConcatenated = std::accumulate(folds.begin(), folds.end(), std::string(""));
      toHash += this->task;
      toHash += this->model;
      toHash += this->inputName;
      toHash += foldsConcatenated;
      toHash += this->timeStamp;
      size_t hashVal = std::hash<std::string>{}(toHash);
      return hashVal;
    }
  };

  /**
    \brief nnUNet segmentation tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT nnUNetTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(nnUNetTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    itkSetMacro(nnUNetDirectory, std::string);
    itkGetConstMacro(nnUNetDirectory, std::string);

    itkSetMacro(ModelDirectory, std::string);
    itkGetConstMacro(ModelDirectory, std::string);

    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

    itkSetMacro(PostProcessingJsonDirectory, std::string);
    itkGetConstMacro(PostProcessingJsonDirectory, std::string);

    itkSetMacro(MixedPrecision, bool);
    itkGetConstMacro(MixedPrecision, bool);
    itkBooleanMacro(MixedPrecision);

    itkSetMacro(Mirror, bool);
    itkGetConstMacro(Mirror, bool);
    itkBooleanMacro(Mirror);

    itkSetMacro(MultiModal, bool);
    itkGetConstMacro(MultiModal, bool);
    itkBooleanMacro(MultiModal);

    itkSetMacro(NoPip, bool);
    itkGetConstMacro(NoPip, bool);
    itkBooleanMacro(NoPip);

    itkSetMacro(Ensemble, bool);
    itkGetConstMacro(Ensemble, bool);
    itkBooleanMacro(Ensemble);

    itkSetMacro(Predict, bool);
    itkGetConstMacro(Predict, bool);
    itkBooleanMacro(Predict);

    itkSetMacro(GpuId, unsigned int);
    itkGetConstMacro(GpuId, unsigned int);

    /**
     * @brief vector of ModelParams.
     * Size > 1 only for ensemble prediction.
     */
    std::vector<ModelParams> m_ParamQ;

    /**
     * @brief Holds paths to other input image modalities.
     *
     */
    std::vector<mitk::Image::ConstPointer> m_OtherModalPaths;

    mitk::Image::ConstPointer m_InputBuffer;

    /**
     * @brief Renders the output LabelSetImage.
     * To called in the main thread.
     */
    void RenderOutputBuffer();

    /**
     * @brief Get the Output Buffer object
     *
     * @return LabelSetImage::Pointer
     */
    LabelSetImage::Pointer GetOutputBuffer();

    /**
     * @brief Sets the outputBuffer to nullptr
     *
     */
    void ClearOutputBuffer();

    /**
     *  @brief Returns the DataStorage from the ToolManager
     */
    mitk::DataStorage *GetDataStorage();

    mitk::DataNode *GetRefNode();

    void SetOutputBuffer(LabelSetImage::Pointer);

  protected:
    /**
     * @brief Construct a new nnUNet Tool object.
     *
     */
    nnUNetTool() = default;

    /**
     * @brief Destroy the nnUNet Tool object and deletes the temp directory.
     *
     */
    ~nnUNetTool();

    /**
     * @brief Overriden method from the tool manager to execute the segmentation
     * Implementation:
     * 1. Saves the inputAtTimeStep in a temporary directory.
     * 2. Copies other modalities, renames and saves in the temporary directory, if required.
     * 3. Sets RESULTS_FOLDER and CUDA_VISIBLE_DEVICES variables in the environment.
     * 3. Iterates through the parameter queue (m_ParamQ) and executes "nnUNet_predict" command with the parameters
     * 4. Expects an output image to be saved in the temporary directory by the python proces. Loads it as
     *    LabelSetImage and sets to previewImage.
     *
     * @param inputAtTimeStep
     * @param oldSegAtTimeStep
     * @param previewImage
     * @param timeStep
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
    void UpdatePrepare() override;

  private:
    std::string m_MitkTempDir;
    std::string m_nnUNetDirectory;
    std::string m_ModelDirectory;
    std::string m_PythonPath;
    std::string m_PostProcessingJsonDirectory;
    // bool m_UseGPU; kept for future
    // bool m_AllInGPU;
    bool m_MixedPrecision;
    bool m_Mirror;
    bool m_NoPip;
    bool m_MultiModal;
    bool m_Ensemble = false;
    bool m_Predict;
    LabelSetImage::Pointer m_OutputBuffer;
    unsigned int m_GpuId;
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
  };
} // namespace mitk
#endif
