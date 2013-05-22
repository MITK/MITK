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

// MITK
#include "mitkOtsuTool3D.h"
#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkRenderingModeProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>

// ITK
#include <itkOtsuMultipleThresholdsImageFilter.h>

// Qt
#include "QMessageBox.h"

#include "mitkRegionGrow3DTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, OtsuTool3D, "Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D()
{
}

mitk::OtsuTool3D::~OtsuTool3D()
{

}

void mitk::OtsuTool3D::Activated()
{
  if (m_ToolManager)
  {
    m_OriginalImageNode = m_ToolManager->GetReferenceData(0);
  }
}

void mitk::OtsuTool3D::Deactivated()
{

}

const char** mitk::OtsuTool3D::GetXPM() const
{
  return mitkRegionGrow3DTool_xpm;
}

void mitk::OtsuTool3D::RunSegmentation(int regions)
{
  //this->m_OtsuSegmentationDialog->setCursor(Qt::WaitCursor);

  int numberOfThresholds = regions - 1;
  int proceed;

  QMessageBox* messageBox = new QMessageBox(QMessageBox::Question, NULL, "The otsu segmentation computation may take several minutes depending on the number of Regions you selected. Proceed anyway?", QMessageBox::Ok | QMessageBox::Cancel);
  if (numberOfThresholds >= 5)
  {
    proceed = messageBox->exec();
    if (proceed != QMessageBox::Ok) return;
  }

  mitk::Image::Pointer mitkImage = 0;

  mitkImage = dynamic_cast<mitk::Image*>( this->m_OriginalImageNode->GetData() );

  try
  {
    // get selected mitk image
    const unsigned short dim = 3;
    typedef short InputPixelType;
    typedef unsigned char OutputPixelType;

    typedef itk::Image< InputPixelType, dim > InputImageType;
    typedef itk::Image< OutputPixelType, dim > OutputImageType;

    typedef itk::OtsuMultipleThresholdsImageFilter< InputImageType, OutputImageType > FilterType;

    FilterType::Pointer filter = FilterType::New();

    filter->SetNumberOfThresholds(numberOfThresholds);

    InputImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    filter->SetInput( itkImage );

    filter->Update();

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    std::string nameOfResultImage = this->m_OriginalImageNode->GetName();
    nameOfResultImage.append("Otsu");
    resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
    resultNode->SetProperty("binary", mitk::BoolProperty::New(false) );
    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );
    resultNode->SetProperty("Image Rendering.Mode", renderingMode);

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();

    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);

    vtkLookupTable *lookupTable = vtkLookupTable::New();
    lookupTable->SetHueRange(1.0, 0.0);
    lookupTable->SetSaturationRange(1.0, 1.0);
    lookupTable->SetValueRange(1.0, 1.0);
    lookupTable->SetTableRange(-1.0, 1.0);
    lookupTable->Build();

    lut->SetVtkLookupTable(lookupTable);

    prop->SetLookupTable(lut);
    resultNode->SetProperty("LookupTable",prop);

    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetRangeMinMax(0, numberOfThresholds);
    levWinProp->SetLevelWindow( levelwindow );
    resultNode->SetProperty( "levelwindow", levWinProp );

    resultNode->SetData( mitk::ImportItkImage ( filter->GetOutput() ) );


    this->m_ToolManager->GetDataStorage()->Add(resultNode, this->m_OriginalImageNode);

    //this->m_OtsuSegmentationDialog->setCursor(Qt::ArrowCursor);

  }
  catch( std::exception& err )
  {
    MITK_ERROR(this->GetNameOfClass()) << err.what();
  }
}

const char* mitk::OtsuTool3D::GetName() const
{
  return "Otsu Segmentation";
}
