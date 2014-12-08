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

#include "mitkCorrectorTool2D.h"
#include "mitkCorrectorAlgorithm.h"

#include "mitkToolManager.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkImageReadAccessor.h"
#include "mitkAbstractTransformGeometry.h"

#include "mitkCorrectorTool2D.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, CorrectorTool2D, "Correction tool");
}

mitk::CorrectorTool2D::CorrectorTool2D(int paintingPixelValue)
:FeedbackContourTool("PressMoveRelease"),
 m_PaintingPixelValue(paintingPixelValue)
{
  GetFeedbackContour()->SetClosed( false ); // don't close the contour to a polygon
}

mitk::CorrectorTool2D::~CorrectorTool2D()
{
}

void mitk::CorrectorTool2D::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION( "Move", OnMouseMoved);
  CONNECT_FUNCTION( "Release", OnMouseReleased);
}

const char** mitk::CorrectorTool2D::GetXPM() const
{
  return mitkCorrectorTool2D_xpm;
}

us::ModuleResource mitk::CorrectorTool2D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Correction_48x48.png");
  return resource;
}

us::ModuleResource mitk::CorrectorTool2D::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Correction_Cursor_32x32.png");
  return resource;
}

const char* mitk::CorrectorTool2D::GetName() const
{
  return "Correction";
}

void mitk::CorrectorTool2D::Activated()
{
  Superclass::Activated();
}

void mitk::CorrectorTool2D::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::CorrectorTool2D::OnMousePressed ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  int timestep = positionEvent->GetSender()->GetTimeStep();
  ContourModel* contour = FeedbackContourTool::GetFeedbackContour();
  contour->Clear();
  contour->Expand(timestep + 1);
  contour->SetClosed(false, timestep);
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  contour->AddVertex( point, timestep );

  FeedbackContourTool::SetFeedbackContourVisible(true);

  return true;
}

bool mitk::CorrectorTool2D::OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();
  ContourModel* contour = FeedbackContourTool::GetFeedbackContour();
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  contour->AddVertex( point, timestep );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::CorrectorTool2D::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );
  if ( !image || !planeGeometry ) return false;

  const AbstractTransformGeometry* abstractTransformGeometry( dynamic_cast<const AbstractTransformGeometry*> (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );
  if ( !image || abstractTransformGeometry ) return false;

  // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
  m_WorkingSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

  if ( m_WorkingSlice.IsNull() )
  {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return false;
  }

  int timestep = positionEvent->GetSender()->GetTimeStep();
  mitk::ContourModel::Pointer singleTimestepContour = mitk::ContourModel::New();

  mitk::ContourModel::VertexIterator it = FeedbackContourTool::GetFeedbackContour()->Begin(timestep);
  mitk::ContourModel::VertexIterator end = FeedbackContourTool::GetFeedbackContour()->End(timestep);

  while(it!=end)
  {
    singleTimestepContour->AddVertex((*it)->Coordinates);
    it++;
  }

  CorrectorAlgorithm::Pointer algorithm = CorrectorAlgorithm::New();
  algorithm->SetInput( m_WorkingSlice );
  algorithm->SetContour( singleTimestepContour );
  try
  {
      algorithm->UpdateLargestPossibleRegion();
  }
  catch ( std::exception& e )
  {
      MITK_ERROR << "Caught exception '" << e.what() << "'" << std::endl;
  }

  mitk::Image::Pointer resultSlice = mitk::Image::New();
  resultSlice->Initialize(algorithm->GetOutput());
  mitk::ImageReadAccessor imAccess(algorithm->GetOutput());
  resultSlice->SetVolume(imAccess.GetData());

  this->WriteBackSegmentationResult(positionEvent, resultSlice);

  return true;
}

