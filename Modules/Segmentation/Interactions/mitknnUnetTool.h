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
#include <MitkSegmentationExports.h>
#include <mitkStandardFileLocations.h>

namespace us //remove?
{
  class ModuleResource;
}

namespace mitk
{
  struct ModelParams
  {
    std::string task;
    std::vector<std::string> folds;
    std::string model;
    std::string trainer;
    std::string planId;
    std::string outputPath;
    std::string outputDir;

    bool operator==(const ModelParams &other) const
    {
      return ((this->task == other.task) && (this->model == other.model));
    }

    size_t generateHash() const
    {
      std::hash<std::string> str_hash;
      std::string toHash = this->task + this->model;
      return str_hash(toHash);
    }
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

    itkSetMacro(UseGPU, bool);
    itkGetConstMacro(UseGPU, bool);
    itkBooleanMacro(UseGPU);

    itkSetMacro(AllInGPU, bool);
    itkGetConstMacro(AllInGPU, bool);
    itkBooleanMacro(AllInGPU);

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

    std::vector<mitk::ModelParams> m_ParamQ;

    itkSetMacro(PreprocessingThreads, unsigned int);
    itkGetConstMacro(PreprocessingThreads, unsigned int);

  protected:
    nnUNetTool();
    ~nnUNetTool() override; // why to override eventhough AutoMLSegmentationWithPreviewTool has no virtual destructor
    //~nnUNetTool() = default; Are there any consequences of not using default?

    LabelSetImage::Pointer ComputeMLPreview(const Image *inputAtTimeStep, TimeStepType timeStep) override;

  private:
    std::string m_MitkTempDir;
    std::string m_nnUNetDirectory;
    std::string m_ModelDirectory;
    std::string m_PythonPath;
    std::string m_PostProcessingJsonDirectory;
    bool m_UseGPU;
    bool m_AllInGPU;
    bool m_MixedPrecision;
    bool m_Mirror; // rename
    bool m_NoPip;
    bool m_MultiModal;
    bool m_Ensemble = false;
    unsigned int m_PreprocessingThreads;
  };
} // namespace mitk

#endif