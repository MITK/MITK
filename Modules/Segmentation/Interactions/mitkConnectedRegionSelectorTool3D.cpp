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

#include "mitkConnectedRegionSelectorTool3D.h"

#include "mitkToolManager.h"
#include "mitkUndoController.h"
#include "mitkRenderingManager.h"
#include "mitkLabelSetImage.h"
#include "mitkImageAccessByItk.h"

#include <itkRegionOfInterestImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, ConnectedRegionSelectorTool3D, "ConnectedRegionSelectorTool3D");
}

mitk::ConnectedRegionSelectorTool3D::ConnectedRegionSelectorTool3D() : SegTool3D("PressMoveReleaseAndPointSetting")
{
  CONNECT_ACTION( AcADDPOINTRMB, AddRegion );
  CONNECT_ACTION( AcADDPOINT, AddRegion );
//  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );
}

mitk::ConnectedRegionSelectorTool3D::~ConnectedRegionSelectorTool3D()
{
}

void mitk::ConnectedRegionSelectorTool3D::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove( m_PreviewNode );
  m_PreviewNode = NULL;
  m_PreviewImage = NULL;
  m_LastEventSender = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::ConnectedRegionSelectorTool3D::Activated()
{
  // feedback node and its visualization properties
  m_PreviewNode = mitk::DataNode::New();
  m_PreviewNode->SetName("preview");

  m_PreviewNode->SetProperty("texture interpolation", BoolProperty::New(false) );
  m_PreviewNode->SetProperty("layer", IntProperty::New(100) );
  m_PreviewNode->SetProperty("binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary shadow", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("helper object", BoolProperty::New(true) );
  m_PreviewNode->SetOpacity(1.0);
  m_PreviewNode->SetColor(0.0, 1.0, 0.0);

  m_ToolManager->GetDataStorage()->Add( m_PreviewNode, m_ToolManager->GetWorkingData(0) );

  m_LastSeedIndex.Fill(0);
}

const char** mitk::ConnectedRegionSelectorTool3D::GetXPM() const
{
  return NULL;//mitkFastMarchingTool_xpm;
}

us::ModuleResource mitk::ConnectedRegionSelectorTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ConnectedRegionSelectorTool3D_48x48.png");
  return resource;
}

us::ModuleResource mitk::ConnectedRegionSelectorTool3D::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ConnectedRegionSelectorTool3D_Cursor_32x32.png");
  return resource;
}

const char* mitk::ConnectedRegionSelectorTool3D::GetName() const
{
  return "Region Selector";
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ConnectedRegionSelectorTool3D::InternalAddRegion( itk::Image<TPixel, VImageDimension>* input )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::ConnectedThresholdImageFilter< ImageType, ImageType > FilterType;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  typename FilterType::Pointer filter = FilterType::New();

  filter->SetInput( input );
  filter->SetLower( m_OverwritePixelValue );
  filter->SetUpper( m_OverwritePixelValue );
  filter->SetConnectivity(FilterType::FaceConnectivity);
  filter->ClearSeeds();
  filter->AddSeed( m_LastSeedIndex );

  m_ProgressCommand->AddStepsToDo(200);
  filter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

  filter->Update();

  m_ProgressCommand->Reset();

  ImageType::Pointer result = filter->GetOutput();
  result->DisconnectPipeline();
/*
   m_PreviewImage = mitk::Image::New();
   mitk::CastToMitkImage(result, m_PreviewImage);
   m_PreviewNode->SetData(m_PreviewImage);
*/
  m_PreviewImage = mitk::Image::New();
  m_PreviewImage->InitializeByItk(result.GetPointer());
  m_PreviewImage->SetChannel(result->GetBufferPointer());

  typename ImageType::RegionType cropRegion;
  cropRegion = result->GetLargestPossibleRegion();

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

bool mitk::ConnectedRegionSelectorTool3D::AddRegion (Action* action, const StateEvent* stateEvent)
{
  if (!stateEvent)
  {
    return false;
  }

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent)
  {
    return false;
  }

  m_LastClickedPosition = positionEvent->GetWorldPosition();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  m_ReferenceGeometry = workingImage->GetGeometry();
  // convert world coordinates to index coordinate on the reference image
  m_ReferenceGeometry->WorldToIndex( m_LastClickedPosition, m_LastSeedIndex);

  m_OverwritePixelValue = workingImage->GetActiveLabelIndex();

  // todo: use it later
//  unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  CurrentlyBusy.Send(true);

  try
  {
    AccessFixedDimensionByItk( workingImage, InternalAddRegion, 3 );
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return false;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return false;
  }

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}
