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
#include "mitkITKImageImport.h"
#include "mitkRenderingManager.h"
#include "mitkRenderingModeProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkIOUtil.h"
#include "mitkLevelWindowManager.h"
#include "mitkImageStatisticsHolder.h"

#include <vtkLookupTable.h>

#include <itkWatershedImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkExceptionObject.h>

//#include <QMessageBox>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, WatershedTool, "Watershed tool");
}


mitk::WatershedTool::WatershedTool() :
  m_Level(0.),
  m_Threshold(0.)
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

  mitk::Image::Pointer output;

  try {
    AccessFixedDimensionByItk_2(input.GetPointer(),ITKWatershed,3,output,0);

    //  m_ToolManager->GetWorkingData(0)->SetData(output);
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(output);

    dataNode->SetProperty("binary", mitk::BoolProperty::New(false));
    dataNode->SetProperty("name", mitk::StringProperty::New("Watershed Result"));
    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );
    dataNode->SetProperty("Image Rendering.Mode", renderingMode);
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
    vtkLookupTable *lookupTable = vtkLookupTable::New();
    lookupTable->SetHueRange(1.0, 0.0);
    lookupTable->SetSaturationRange(1.0, 1.0);
    lookupTable->SetValueRange(1.0, 1.0);
    lookupTable->SetTableRange(-1.0, 1.0);
    lookupTable->Build();
    lookupTable->SetTableValue(1,0,0,0);

    lut->SetVtkLookupTable(lookupTable);
    prop->SetLookupTable(lut);
    dataNode->SetProperty("LookupTable",prop);
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetRangeMinMax(0, output->GetStatistics()->GetScalarValueMax());
    levWinProp->SetLevelWindow( levelwindow );
    dataNode->SetProperty( "levelwindow", levWinProp );
    dataNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5));

    m_ToolManager->GetDataStorage()->Add(dataNode);
  }
  catch(itk::ExceptionObject& e)
  {
//    QMessageBox::warning(NULL,QString::fromAscii("Watershed Filter Error"), QString::fromAscii(e.GetDescription()));
    MITK_ERROR<<"Watershed Filter Error";
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);

}

template <typename TPixel, unsigned int VImageDimension>
void mitk::WatershedTool::ITKWatershed( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image::Pointer& segmentation, unsigned int timeStep )
{
  typedef itk::WatershedImageFilter< itk::Image<float, VImageDimension> > WatershedFilter;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< itk::Image<TPixel, VImageDimension >, itk::Image<float, VImageDimension> > MagnitudeFilter;

  typename MagnitudeFilter::Pointer magnitude = MagnitudeFilter::New();
  magnitude->SetInput(originalImage);
  magnitude->SetSigma(1.0);
  //magnitude->Update();

  typename WatershedFilter::Pointer watershed = WatershedFilter::New();
  watershed->SetInput(magnitude->GetOutput());
  watershed->SetThreshold(m_Threshold);
  watershed->SetLevel(m_Level);
  //watershed->Update();

  typedef itk::CastImageFilter<typename WatershedFilter::OutputImageType, itk::Image<Tool::DefaultSegmentationDataType,3> > CastFilter;
  typename CastFilter::Pointer cast = CastFilter::New();
  cast->SetInput(watershed->GetOutput());
  cast->Update();

  segmentation = mitk::GrabItkImageMemory(cast->GetOutput());

}
