/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKOTSUTOOL3D_H
#define MITKOTSUTOOL3D_H

#include "itkImage.h"
#include "mitkAutoSegmentationTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class Image;

  class MITKSEGMENTATION_EXPORT OtsuTool3D : public AutoSegmentationTool
  {
  public:
    mitkClassMacro(OtsuTool3D, AutoSegmentationTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    void RunSegmentation(int regions, bool useValley, int numberOfBins);
    void ConfirmSegmentation();
    // void UpdateBinaryPreview(int regionID);
    void UpdateBinaryPreview(std::vector<int> regionIDs);
    void UpdateVolumePreview(bool volumeRendering);
    void ShowMultiLabelResultNode(bool);

    int GetNumberOfBins();

  protected:
    OtsuTool3D();
    ~OtsuTool3D() override;

    template <typename TPixel, unsigned int VImageDimension>
    void CalculatePreview(itk::Image<TPixel, VImageDimension> *itkImage, std::vector<int> regionIDs);

    itk::SmartPointer<Image> m_OriginalImage;
    // holds the user selected binary segmentation
    mitk::DataNode::Pointer m_BinaryPreviewNode;
    // holds the multilabel result as a preview image
    mitk::DataNode::Pointer m_MultiLabelResultNode;
    // holds the user selected binary segmentation masked original image
    mitk::DataNode::Pointer m_MaskedImagePreviewNode;

  }; // class
} // namespace
#endif
