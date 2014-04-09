/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef MITKPIXELMANIPULATIONTOOL_H
#define MITKPIXELMANIPULATIONTOOL_H

#include "mitkTool.h"
#include <MitkSegmentationExports.h>

#include "itkImage.h"

namespace mitk
{
  class Image;

  class MitkSegmentation_EXPORT PixelManipulationTool : public Tool
  {
  public:
    mitkClassMacro(PixelManipulationTool, Tool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetValue( int value );
    int GetValue();

    void SetFixedValue( int value );
    int GetFixedValue();

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void CalculateImage();


  protected:
    PixelManipulationTool();
    virtual ~PixelManipulationTool();

    virtual void OnRoiDataChanged();
    void AddImageToDataStorage(itk::SmartPointer<mitk::Image> image);

    template <typename TPixel, unsigned int VImageDimension>
    void ITKPixelManipulation( itk::Image<TPixel, VImageDimension>* originalImage, Image* maskImage, Image* newImage, int newValue);

    mitk::DataNode::Pointer m_OriginalImageNode;
    int m_Value;
    bool m_FixedValue;
  };//clas
}//namespace
#endif
