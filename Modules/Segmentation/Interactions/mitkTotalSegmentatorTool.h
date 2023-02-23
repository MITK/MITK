/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKTOTALSEGMENTATORTOOL_H
#define MITKTOTALSEGMENTATORTOOL_H

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include "mitkProcessExecutor.h"


namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class MITKSEGMENTATION_EXPORT TotalSegmentatorTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(TotalSegmentatorTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    void Activated() override;

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string); 
    
    itkSetMacro(SubTask, std::string);
    itkGetConstMacro(SubTask, std::string);
    
    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(GpuId, unsigned int);
    itkGetConstMacro(GpuId, unsigned int);

    itkSetMacro(Fast, bool);
    itkGetConstMacro(Fast, bool);
    itkBooleanMacro(Fast);

  protected:
    TotalSegmentatorTool();
    ~TotalSegmentatorTool();

    /**
     * @brief Overriden method from the tool manager to execute the segmentation
     * Implementation:
     * 1. Saves the inputAtTimeStep in a temporary directory.
     * 3. Sets CUDA_VISIBLE_DEVICES variables in the environment.
     * 3. Executes "TotalSegmentator" command with the parameters
     * 4. Expects an output image to be saved in the temporary directory by the python proces. Loads it as
     *    LabelSetImage and sets to previewImage.
     *
     * @param inputAtTimeStep
     * @param oldSegAtTimeStep
     * @param previewImage
     * @param timeStep
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
   
  private:
    void run_totalsegmentator(ProcessExecutor::Pointer,const std::string &, const std::string &, bool, bool, unsigned int, const std::string &);

    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_SubTask = "total";
    unsigned int m_GpuId;
    bool m_Fast;
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string m_DEFAULT_TOTAL_TASK = "total";
  }; // class
} // namespace
#endif
