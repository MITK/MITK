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

#include "mitkSplitConnectedRegionsTool3D.h"

#include "mitkLabelSetImage.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"

// itk
#include <itkBinaryThresholdImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkAutoCropLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkBinaryShapeKeepNObjectsImageFilter.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkRelabelLabelMapFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, SplitConnectedRegionsTool3D, "SplitConnectedRegionsTool3D");
}

mitk::SplitConnectedRegionsTool3D::SplitConnectedRegionsTool3D() : SegTool3D("dummy"),
m_NumberOfConnectedRegionsToKeep(1),
m_ColorSequenceRainbow( new mitk::ColorSequenceRainbow())
{
//  m_ColorSequenceRainbow->GoToBegin();
}

mitk::SplitConnectedRegionsTool3D::~SplitConnectedRegionsTool3D()
{
  delete m_ColorSequenceRainbow;
}

const char** mitk::SplitConnectedRegionsTool3D::GetXPM() const
{
  return NULL;//mitkSplitConnectedRegionsTool3D_xpm;
}

us::ModuleResource mitk::SplitConnectedRegionsTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  assert(module);
  us::ModuleResource resource = module->GetResource("SplitConnectedRegionsTool3D_48x48.png");
  return resource;
}

const char* mitk::SplitConnectedRegionsTool3D::GetName() const
{
  return "Split";
}

void mitk::SplitConnectedRegionsTool3D::SetNumberOfConnectedRegionsToKeep(int value)
{
  m_NumberOfConnectedRegionsToKeep = value;
}

void mitk::SplitConnectedRegionsTool3D::AcceptPreview()
{
  if (m_PreviewImage.IsNull()) return;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  CurrentlyBusy.Send(true);

  try
  {
    AccessTwoImagesFixedDimensionByItk( workingImage, m_PreviewImage, InternalAcceptPreview, 3);
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  workingImage->Modified();

  m_PreviewNode->SetVisibility(true);

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SplitConnectedRegionsTool3D::Run()
{
   //  this->InitializeUndoController();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabel()->GetPixelValue();

  m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(workingImage, InternalRun);
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  CurrentlyBusy.Send(false);

  workingImage->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

template <typename ImageType1, typename ImageType2>
void mitk::SplitConnectedRegionsTool3D::InternalAcceptPreview( ImageType1* targetImage, const ImageType2* sourceImage )
{
  typedef itk::BinaryImageToShapeLabelMapFilter< ImageType2 > BinaryImageToShapeLabelMapFilterType;
  typedef itk::RelabelLabelMapFilter< typename BinaryImageToShapeLabelMapFilterType::OutputImageType > RelabelFilterType;
  typedef itk::LabelMapToLabelImageFilter< typename BinaryImageToShapeLabelMapFilterType::OutputImageType, ImageType2 > LabelMap2ImageType;

  typename BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
  binaryImageToShapeLabelMapFilter->SetInput( sourceImage );
  binaryImageToShapeLabelMapFilter->SetOutputBackgroundValue(0);
  binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);

  typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput( binaryImageToShapeLabelMapFilter->GetOutput() );
  relabelFilter->SetInPlace(true);

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( relabelFilter->GetOutput() );

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->AddStepsToDo(200);
    binaryImageToShapeLabelMapFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
    relabelFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  }

  label2image->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename ImageType2::Pointer result = label2image->GetOutput();
  result->DisconnectPipeline();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  std::string name = workingImage->GetActiveLabel()->GetName();

  int numberOfObjects = binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects();

  int maxLabel = workingImage->GetNumberOfLabels();

  std::string originalName = workingImage->GetActiveLabel()->GetName();

  // Loop over all blobs
  for (unsigned int i=0; i<numberOfObjects; i++)
  {
    mitk::Color color = m_ColorSequenceRainbow->GetNextColor();
    std::stringstream stream;
    stream << originalName << "-sp-" << i;
    std::string name = stream.str().c_str();
    workingImage->GetLabelSet()->AddLabel(name, color);
  }

  typedef itk::ImageRegionIterator< ImageType1 >      TargetIteratorType;
  typedef itk::ImageRegionConstIterator< ImageType2 > SourceIteratorType;

  typename ImageType1::IndexType cropIndex;
  cropIndex[0] = m_RequestedRegion.GetIndex(0);
  cropIndex[1] = m_RequestedRegion.GetIndex(1);
  cropIndex[2] = m_RequestedRegion.GetIndex(2);

  typename ImageType1::SizeType cropSize;
  cropSize[0] = m_RequestedRegion.GetSize(0);
  cropSize[1] = m_RequestedRegion.GetSize(1);
  cropSize[2] = m_RequestedRegion.GetSize(2);

  typename ImageType1::RegionType cropRegion(cropIndex,cropSize);

  SourceIteratorType sourceIter( result, result->GetLargestPossibleRegion() );
  TargetIteratorType targetIter( targetImage, cropRegion );

  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while ( !targetIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if (sourceValue)
    {
      targetIter.Set( maxLabel + sourceValue - 1);
    }

    ++sourceIter;
    ++targetIter;
  }

}

template < typename TPixel, unsigned int VDimension >
void mitk::SplitConnectedRegionsTool3D::InternalRun( itk::Image<TPixel, VDimension>* input )
{
  typedef itk::Image< TPixel, VDimension > ImageType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;
  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
  typedef itk::ShapeLabelObject< TPixel, VDimension > ShapeLabelObjectType;
  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > Image2LabelMapType;
  typedef itk::AutoCropLabelMapFilter< LabelMapType > AutoCropType;
  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMap2ImageType;
  typedef itk::BinaryShapeKeepNObjectsImageFilter< ImageType > BinaryShapeKeepNObjectsImageFilterType;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput( input );
  thresholdFilter->SetLowerThreshold(m_PaintingPixelValue);
  thresholdFilter->SetUpperThreshold(m_PaintingPixelValue);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);

  typename Image2LabelMapType::Pointer image2label = Image2LabelMapType::New();
  image2label->SetInput(thresholdFilter->GetOutput());

  typename AutoCropType::SizeType border;
  border[0] = 3;
  border[1] = 3;
  border[2] = 3;

  typename AutoCropType::Pointer autoCropFilter = AutoCropType::New();
  autoCropFilter->SetInput( image2label->GetOutput() );
  autoCropFilter->SetCropBorder(border);
  autoCropFilter->InPlaceOn();

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( autoCropFilter->GetOutput() );

  typename BinaryShapeKeepNObjectsImageFilterType::Pointer binaryShapeKeepNObjectsImageFilter = BinaryShapeKeepNObjectsImageFilterType::New();
  binaryShapeKeepNObjectsImageFilter->SetInput( label2image->GetOutput() );
  binaryShapeKeepNObjectsImageFilter->SetBackgroundValue( 0 );
  binaryShapeKeepNObjectsImageFilter->SetNumberOfObjects( m_NumberOfConnectedRegionsToKeep );
  binaryShapeKeepNObjectsImageFilter->SetAttribute( ShapeLabelObjectType::PHYSICAL_SIZE);
  binaryShapeKeepNObjectsImageFilter->SetForegroundValue( 1 );
//  binaryShapeKeepNObjectsImageFilter->ReleaseDataFlagOn();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->AddStepsToDo(200);
    thresholdFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
    binaryShapeKeepNObjectsImageFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  }

  binaryShapeKeepNObjectsImageFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename ImageType::Pointer result = binaryShapeKeepNObjectsImageFilter->GetOutput();
  result->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage(result.GetPointer(), m_PreviewImage);

  typename ImageType::RegionType cropRegion;
  cropRegion = autoCropFilter->GetOutput()->GetLargestPossibleRegion();

  const typename ImageType::SizeType& cropSize = cropRegion.GetSize();
  const typename ImageType::IndexType& cropIndex = cropRegion.GetIndex();

  mitk::SlicedGeometry3D* slicedGeometry = m_PreviewImage->GetSlicedGeometry();
  mitk::Point3D origin;
  vtk2itk(cropIndex, origin);
  workingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  slicedGeometry->SetOrigin(origin);

  m_PreviewNode->SetData(m_PreviewImage);

  m_RequestedRegion = workingImage->GetLargestPossibleRegion();

  m_RequestedRegion.SetIndex(0,cropIndex[0]);
  m_RequestedRegion.SetIndex(1,cropIndex[1]);
  m_RequestedRegion.SetIndex(2,cropIndex[2]);

  m_RequestedRegion.SetSize(0,cropSize[0]);
  m_RequestedRegion.SetSize(1,cropSize[1]);
  m_RequestedRegion.SetSize(2,cropSize[2]);
}
