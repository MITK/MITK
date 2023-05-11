/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkOtsuTool3D_h
#define mitkOtsuTool3D_h

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class Image;

  class MITKSEGMENTATION_EXPORT OtsuTool3D : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(OtsuTool3D, SegWithPreviewTool);
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
    OtsuTool3D();
    ~OtsuTool3D() = default;

    void UpdatePrepare() override;
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

    unsigned int m_NumberOfBins = 128;
    unsigned int m_NumberOfRegions = 2;
    bool m_UseValley = false;
  }; // class
} // namespace
#endif
