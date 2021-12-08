/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnUnetTool_h_Included
#define mitknnUnetTool_h_Included

#include "mitkAutoMLSegmentationWithPreviewTool.h"
#include "mitkCommon.h"
#include "mitkToolManager.h"
#include <MitkSegmentationExports.h>
#include <mitkStandardFileLocations.h>
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
  };

  /**
    \brief nnUNet segmentation tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT nnUNetTool : public AutoMLSegmentationWithPreviewTool
  {
  public:
    mitkClassMacro(nnUNetTool, AutoMLSegmentationWithPreviewTool);
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

    std::pair<const Image *, LabelSetImage::Pointer> m_InputOutputPair;

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

  protected:
    /**
     * @brief Construct a new nnUNet Tool object and temp directory.
     *
     */
    nnUNetTool();

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
     *    LabelSetImage and returns.
     *
     * @param inputAtTimeStep
     * @param timeStep
     * @return LabelSetImage::Pointer
     */
    LabelSetImage::Pointer ComputeMLPreview(const Image *inputAtTimeStep, TimeStepType timeStep) override;
    void UpdateCleanUp() override;
    void SetNodeProperties(LabelSetImage::Pointer) override;

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
  };
} // namespace mitk
#endif
