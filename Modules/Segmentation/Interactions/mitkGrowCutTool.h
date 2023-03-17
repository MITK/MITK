/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGrowCutTool_h
#define mitkGrowCutTool_h

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class MITKSEGMENTATION_EXPORT GrowCutTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(GrowCutTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkSetMacro(DistancePenalty, double);
    itkGetConstMacro(DistancePenalty, double);

    typedef itk::Image<DefaultSegmentationDataType, 3> SeedImageType;
    typedef typename SeedImageType::IndexType IndexType;

    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    bool SeedImageIsValid();

  protected:
    GrowCutTool(); // purposely hidden
    ~GrowCutTool() override;

    void DoUpdatePreview(const Image *inputAtTimeStep,
                         const Image *oldSegAtTimeStep,
                         LabelSetImage *previewImage,
                         TimeStepType timeStep) override;

    double m_DistancePenalty = 0.0;
  };

} // namespace mitk

#endif
