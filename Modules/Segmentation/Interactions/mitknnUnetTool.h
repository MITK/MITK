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

namespace us
{
  class ModuleResource;
}

namespace mitk
{
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

    const char** GetXPM() const override;
    const char* GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    itkSetMacro(Model, std::string);
    itkGetConstMacro(Model, std::string);

    itkSetMacro(Task, std::string);
    itkGetConstMacro(Task, std::string);

    itkSetMacro(Mode, std::string);
    itkGetConstMacro(Mode, std::string);

    itkSetMacro(nnUNetDirectory, std::string);
    itkGetConstMacro(nnUNetDirectory, std::string);

    itkSetMacro(ModelDirectory, std::string);
    itkGetConstMacro(ModelDirectory, std::string);
    
    itkSetMacro(OutputDirectory, std::string);
    itkGetConstMacro(OutputDirectory, std::string);

    itkSetMacro(UseGPU, bool);
    itkGetConstMacro(UseGPU, bool);
    itkBooleanMacro(UseGPU);

    itkSetMacro(LowRes, bool);
    itkGetConstMacro(LowRes, bool);
    itkBooleanMacro(LowRes);

    itkSetMacro(AllInGPU, bool);
    itkGetConstMacro(AllInGPU, bool);
    itkBooleanMacro(AllInGPU);

    itkSetMacro(MixedPrecision, bool);
    itkGetConstMacro(MixedPrecision, bool);
    itkBooleanMacro(MixedPrecision);

    itkSetMacro(ExportSegmentation, bool);
    itkGetConstMacro(ExportSegmentation, bool);
    itkBooleanMacro(ExportSegmentation);

    itkSetMacro(Mirror, bool);
    itkGetConstMacro(Mirror, bool);
    itkBooleanMacro(Mirror);

    itkSetMacro(PreprocessingThreads, unsigned int);
    itkGetConstMacro(PreprocessingThreads, unsigned int);


  protected:
    nnUNetTool();
    ~nnUNetTool() = default;

    LabelSetImage::Pointer ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType timeStep) override;

  private:
    std::string m_Model;
    std::string m_Mode;
    std::string m_Task;
    std::string m_nnUNetDirectory;
    std::string m_ModelDirectory;
    std::string m_OutputDirectory;
    bool m_UseGPU;
    bool m_LowRes;
    bool m_AllInGPU;
    bool m_MixedPrecision;
    bool m_ExportSegmentation;
    bool m_Mirror; //rename
    unsigned int m_PreprocessingThreads;
    
    const std::string m_PythonProjectPath;
    const std::string m_PythonFileName;
    const std::string m_InputImageVarName;
    const std::string m_OutputImageVarName;
  };
} // namespace
#endif