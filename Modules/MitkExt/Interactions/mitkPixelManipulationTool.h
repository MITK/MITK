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
#include "MitkExtExports.h"

#include "itkImage.h"

namespace mitk
{
  class MitkExt_EXPORT PixelManipulationTool : public Tool
  {
  public:
    mitkClassMacro(PixelManipulationTool, Tool);
    itkNewMacro(PixelManipulationTool);

    itkSetMacro(Value, int);
    itkGetMacro(Value, int);

    itkSetMacro(FixedValue, bool);
    itkGetMacro(FixedValue, bool);
    itkBooleanMacro(FixedValue);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void CalculateImage();


  protected:
    PixelManipulationTool();
    virtual ~PixelManipulationTool();

    virtual void OnRoiDataChanged();
    void AddImageToDataStorage(mitk::Image::Pointer image);

    template <typename TPixel, unsigned int VImageDimension>
    void ITKPixelManipulation( itk::Image<TPixel, VImageDimension>* originalImage, Image* maskImage, Image* newImage, int newValue);

    mitk::DataNode::Pointer m_OriginalImageNode;
    int m_Value;
    bool m_FixedValue;
  };//clas
}//namespace
#endif
