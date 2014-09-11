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
#include "mitkPixelManipulationTool.h"

#include "mitkToolManager.h"
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkProperties.h"
#include "mitkBoundingObjectToSegmentationFilter.h"
#include <itkImageRegionIterator.h>

#include "mitkPixelManipulationTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, PixelManipulationTool, "Pixel manipulation tool");
}

mitk::PixelManipulationTool::PixelManipulationTool() : Tool("dummy"),
m_Value(0),
m_FixedValue(false)
{
}

mitk::PixelManipulationTool::~PixelManipulationTool()
{
}

void mitk::PixelManipulationTool::Activated()
{
  m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::PixelManipulationTool> (this,&mitk::PixelManipulationTool::OnRoiDataChanged);
  m_OriginalImageNode = m_ToolManager->GetReferenceData(0);

  if (m_OriginalImageNode.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData());
    if ( image.IsNotNull())
    {
      //mitk::ScalarType scalar = image->GetScalarValueMax();
    }
  }
  else
    m_ToolManager->ActivateTool(-1);
}

void mitk::PixelManipulationTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::PixelManipulationTool> (this,&mitk::PixelManipulationTool::OnRoiDataChanged);

}

const char* mitk::PixelManipulationTool::GetName() const
{
  return "pixelmanipulation";
}

const char** mitk::PixelManipulationTool::GetXPM() const
{
  return mitkPixelManipulationTool_xpm;
}

void mitk::PixelManipulationTool::OnRoiDataChanged()
{

}

void mitk::PixelManipulationTool::CalculateImage()
{
  if (m_OriginalImageNode.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData());
    mitk::DataNode* maskNode = m_ToolManager->GetRoiData(0);
    mitk::Image::Pointer roi = mitk::Image::New();

    if (maskNode)
    {
      mitk::BoundingObject* boundingObject = dynamic_cast<mitk::BoundingObject*> (maskNode->GetData());

      if (boundingObject)
      {
        mitk::BoundingObjectToSegmentationFilter::Pointer filter = mitk::BoundingObjectToSegmentationFilter::New();
        filter->SetBoundingObject( boundingObject);
        filter->SetInput(image);
        filter->Update();
        roi = filter->GetOutput();
      }
      else
        roi =  dynamic_cast<mitk::Image*> (maskNode->GetData());

      mitk::Image::Pointer newImage = mitk::Image::New();
      newImage->Initialize(image);

      if (image)
      {
        AccessByItk_3(image, ITKPixelManipulation, roi, newImage, m_Value);
        this->AddImageToDataStorage(newImage);
      }
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::PixelManipulationTool::ITKPixelManipulation( itk::Image<TPixel, VImageDimension>* originalImage, Image* maskImage, Image* newImage, int newValue)
{

  typedef itk::Image< TPixel, VImageDimension> itkImageType;
  typedef itk::Image< unsigned char, 3> itkMaskType;
  typename itkImageType::Pointer itkImage;
  typename itkMaskType::Pointer itkMask;
  CastToItkImage( newImage, itkImage);
  CastToItkImage( maskImage, itkMask);

  typedef itk::ImageRegionConstIterator< itkImageType >     InputIteratorType;
  typedef itk::ImageRegionIterator< itkImageType >     OutputIteratorType;
  typedef itk::ImageRegionConstIterator< itkMaskType > MaskIteratorType;

  MaskIteratorType maskIterator ( itkMask, itkMask->GetLargestPossibleRegion() );
  InputIteratorType inputIterator( originalImage, originalImage->GetLargestPossibleRegion() );
  OutputIteratorType outputIterator( itkImage, itkImage->GetLargestPossibleRegion() );

  inputIterator.GoToBegin();
  outputIterator.GoToBegin();
  maskIterator.GoToBegin();

  while (!outputIterator.IsAtEnd())
  {
    if (maskIterator.Get())
    {
      if (m_FixedValue)
        outputIterator.Set(newValue);
      else
        outputIterator.Set( inputIterator.Get()+ newValue);
    }
    else
      outputIterator.Set( inputIterator.Get());

    ++inputIterator;
    ++outputIterator;
    ++maskIterator;
  }
}

void mitk::PixelManipulationTool::AddImageToDataStorage(mitk::Image::Pointer image)
{
  if (image.IsNotNull())
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    std::string name = m_OriginalImageNode->GetName();
    name.append("_modified");
    node->SetName(name);
    node->SetProperty("binary", mitk::BoolProperty::New(false));
    node->SetData(image);

    if (m_ToolManager)
      m_ToolManager->GetDataStorage()->Add(node, m_OriginalImageNode);
  }
}

void mitk::PixelManipulationTool::SetValue( int value )
{
  m_Value = value;
}

int mitk::PixelManipulationTool::GetValue()
{
  return m_Value;
}

void mitk::PixelManipulationTool::SetFixedValue( int value )
{
  m_FixedValue = value;
}

int mitk::PixelManipulationTool::GetFixedValue()
{
  return m_FixedValue;
}
