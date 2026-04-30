/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkOtsuTool3D_h
#define mitkOtsuTool3D_h

#include <mitkSegWithPreviewTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class Image;

  /**
   * \brief Otsu multi-threshold 3D segmentation tool with preview.
   *
   * Uses the Otsu method to automatically determine multiple thresholds
   * for segmenting the reference image into distinct regions. The number
   * of regions, histogram bins, and valley emphasis can be configured.
   *
   * \ingroup ToolManagerEtAl
   * \sa OtsuSegmentationFilter, SegWithPreviewTool
   */
  class MITKSEGMENTATION_EXPORT OtsuTool3D : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(OtsuTool3D, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
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
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;

    unsigned int m_NumberOfBins = 128;
    unsigned int m_NumberOfRegions = 2;
    bool m_UseValley = false;
  }; // class
} // namespace
#endif
