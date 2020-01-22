/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKPIXELMANIPULATIONTOOL_H
#define MITKPIXELMANIPULATIONTOOL_H

#include "mitkTool.h"
#include <MitkSegmentationExports.h>

#include "itkImage.h"

namespace mitk
{
  class Image;

  class MITKSEGMENTATION_EXPORT PixelManipulationTool : public Tool
  {
  public:
    mitkClassMacro(PixelManipulationTool, Tool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void SetValue(int value);
    int GetValue();

    void SetFixedValue(int value);
    int GetFixedValue();

    const char *GetName() const override;
    const char **GetXPM() const override;

    void Activated() override;
    void Deactivated() override;

    virtual void CalculateImage();

  protected:
    PixelManipulationTool();
    ~PixelManipulationTool() override;

    virtual void OnRoiDataChanged();
    void AddImageToDataStorage(itk::SmartPointer<mitk::Image> image);

    template <typename TPixel, unsigned int VImageDimension>
    void ITKPixelManipulation(itk::Image<TPixel, VImageDimension> *originalImage,
                              Image *maskImage,
                              Image *newImage,
                              int newValue);

    mitk::DataNode::Pointer m_OriginalImageNode;
    int m_Value;
    bool m_FixedValue;
  }; // clas
} // namespace
#endif
