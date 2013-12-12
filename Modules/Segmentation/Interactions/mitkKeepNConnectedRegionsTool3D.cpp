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

#include "mitkKeepNConnectedRegionsTool3D.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkAutoCropLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>

#include <itkBinaryShapeKeepNObjectsImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, KeepNConnectedRegionsTool3D, "KeepNConnectedRegions3D tool");
}

mitk::KeepNConnectedRegionsTool3D::KeepNConnectedRegionsTool3D()
: m_NumberOfConnectedRegionsToKeep(1)
{
}

mitk::KeepNConnectedRegionsTool3D::~KeepNConnectedRegionsTool3D()
{
}

const char** mitk::KeepNConnectedRegionsTool3D::GetXPM() const
{
  return NULL;//mitkKeepNConnectedRegionsTool3D_xpm;
}

us::ModuleResource mitk::KeepNConnectedRegionsTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  assert(module);
  us::ModuleResource resource = module->GetResource("KeepNConnectedRegionsTool3D_48x48.png");
  return resource;
}

const char* mitk::KeepNConnectedRegionsTool3D::GetName() const
{
  return "Keep N Largest";
}

void mitk::KeepNConnectedRegionsTool3D::SetNumberOfConnectedRegionsToKeep(int value)
{
  m_NumberOfConnectedRegionsToKeep = value;
}

void mitk::KeepNConnectedRegionsTool3D::Run()
{
   //  this->InitializeUndoController();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  m_OverwritePixelValue = workingImage->GetActiveLabelIndex();

  // todo: use it later
  //unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(workingImage, InternalProcessing);
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

template < typename TPixel, unsigned int VDimension >
void mitk::KeepNConnectedRegionsTool3D::InternalProcessing( itk::Image< TPixel, VDimension>* input )
{
  typedef itk::Image< TPixel, VDimension > ImageType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;
  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
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
  thresholdFilter->SetLowerThreshold(m_OverwritePixelValue);
  thresholdFilter->SetUpperThreshold(m_OverwritePixelValue);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->ReleaseDataFlagOn();

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
//  binaryShapeKeepNObjectsImageFilter->SetAttribute( BinaryShapeKeepNObjectsImageFilterType::LabelObjectType::SIZE);
  binaryShapeKeepNObjectsImageFilter->SetForegroundValue( 1 );
  binaryShapeKeepNObjectsImageFilter->ReleaseDataFlagOn();

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
  m_PreviewImage->InitializeByItk(result.GetPointer());
  m_PreviewImage->SetChannel(result->GetBufferPointer());

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
