/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKOTSUTOOL3D_H
#define MITKOTSUTOOL3D_H

#include "mitkAutoMLSegmentationWithPreviewTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class Image;

  class MITKSEGMENTATION_EXPORT OtsuTool3D : public AutoMLSegmentationWithPreviewTool
  {
  public:
    mitkClassMacro(OtsuTool3D, AutoMLSegmentationWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    itkSetMacro(NumberOfBins, unsigned int);
    itkGetConstMacro(NumberOfBins, unsigned int);

    itkSetMacro(NumberOfRegions, unsigned int);
    itkGetConstMacro(NumberOfRegions, unsigned int);

    itkSetMacro(UseValley, bool);
    itkGetConstMacro(UseValley, bool);
    itkBooleanMacro(UseValley);

    /**Returns the number of max bins based on the current input image.*/
    unsigned int GetMaxNumberOfBins() const;

  protected:
    OtsuTool3D() = default;
    ~OtsuTool3D() = default;

    LabelSetImage::Pointer ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType timeStep) override;

    unsigned int m_NumberOfBins = 128;
    unsigned int m_NumberOfRegions = 2;
    bool m_UseValley = false;
  }; // class
} // namespace
#endif
