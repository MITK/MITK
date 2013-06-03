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

#include "mitkWatershedTool.h"

#include "mitkBinaryThresholdTool.xpm"
#include "mitkToolManager.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

#include <itkWatershedImageFilter.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, WatershedTool, "Watershed tool");
}


mitk::WatershedTool::WatershedTool()
{
}

mitk::WatershedTool::~WatershedTool()
{
}

void mitk::WatershedTool::Activated()
{
  Superclass::Activated();
}

void mitk::WatershedTool::Deactivated()
{
  Superclass::Deactivated();
}

const char** mitk::WatershedTool::GetXPM() const
{
  return mitkBinaryThresholdTool_xpm;
}

const char* mitk::WatershedTool::GetName() const
{
  return "Watershedding";
}

void mitk::WatershedTool::DoIt()
{

  mitk::Image::Pointer input = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());

  mitk::Image::Pointer output = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());

  AccessFixedDimensionByItk_2(input.GetPointer(),ITKWatershed,3,output,0);

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

}

template <typename TPixel, unsigned int VImageDimension>
void mitk::WatershedTool::ITKWatershed( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep )
{
  typedef itk::WatershedImageFilter< itk::Image<TPixel, VImageDimension> > WatershedFilter;

  typename WatershedFilter::OutputImageType::Pointer output;

  typename WatershedFilter::Pointer watershed = WatershedFilter::New();
  watershed->SetInput(originalImage);
  watershed->Update();

  output = watershed->GetOutput();

  typedef itk::CastImageFilter<typename WatershedFilter::OutputImageType, itk::Image<Tool::DefaultSegmentationDataType,3> > CastFilter;
  typename CastFilter::Pointer cast = CastFilter::New();
  cast->SetInput(output);
  cast->Update();

  itk::Image<Tool::DefaultSegmentationDataType,3>::Pointer out = cast->GetOutput();
  CastToItkImage( segmentation, out );


}
