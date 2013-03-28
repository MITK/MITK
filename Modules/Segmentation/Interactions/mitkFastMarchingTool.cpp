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

#include "mitkFastMarchingTool.h"
#include "mitkToolManager.h"


#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkApplicationCursor.h"

#include "mitkFastMarchingTool.xpm"

#include "mitkInteractionConst.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool, "FastMarching tool");
}


mitk::FastMarchingTool::FastMarchingTool()
:FeedbackContourTool("PressMoveReleaseAndPointSetting"),
m_LowerThreshold(200),
m_UpperThreshold(200),
m_InitialLowerThreshold(200),
m_InitialUpperThreshold(200),
m_ScreenYDifference(0),
m_MouseDistanceScaleFactor(0.5)
{
  CONNECT_ACTION( AcNEWPOINT, OnMousePressed );
  CONNECT_ACTION( AcMOVEPOINT, OnMouseMoved );
  CONNECT_ACTION( AcFINISHMOVEMENT, OnMouseReleased );
  CONNECT_ACTION( AcADDPOINTRMB, OnAddPoint );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );
}

mitk::FastMarchingTool::~FastMarchingTool()
{
}

const char** mitk::FastMarchingTool::GetXPM() const
{
  return mitkFastMarchingTool_xpm;
}

const char* mitk::FastMarchingTool::GetName() const
{
  return "FastMarching";
}

void mitk::FastMarchingTool::Activated()
{
  Superclass::Activated();
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();
}


bool mitk::FastMarchingTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( FeedbackContourTool::CanHandleEvent(stateEvent) > 0.0 )
  {
    if (positionEvent)
    {


      m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice( positionEvent );
      m_WorkingSlice   = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );

      if ( m_WorkingSlice.IsNotNull() )
      {
        // 2. Determine if the user clicked inside or outside of the segmentation
        const Geometry3D* workingSliceGeometry = m_WorkingSlice->GetGeometry();
        Point3D mprojectedPointIn2D;
        workingSliceGeometry->WorldToIndex( positionEvent->GetWorldPosition(), mprojectedPointIn2D);
        itk::Index<2> projectedPointInWorkingSlice2D;
        projectedPointInWorkingSlice2D[0] = static_cast<int>( mprojectedPointIn2D[0] - 0.5 );
        projectedPointInWorkingSlice2D[1] = static_cast<int>( mprojectedPointIn2D[1] - 0.5 );

        if ( workingSliceGeometry->IsIndexInside( projectedPointInWorkingSlice2D ) )
        {
          if ( m_ReferenceSlice.IsNotNull() )
          {
            MITK_INFO << "HeyHo";
          }
        }
      }
    }
  }

  return true;
}



/**
If in region growing mode (m_ReferenceSlice != NULL), then
1. Calculate the new thresholds from mouse position (relative to first position)
2. Perform a new region growing and update the feedback contour
*/
bool mitk::FastMarchingTool::OnMouseMoved(Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) > 0.0 )
  {
    if ( m_ReferenceSlice.IsNotNull() )
    {
      const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
      if (positionEvent)
      {
        ApplicationCursor* cursor = ApplicationCursor::GetInstance();
        if (!cursor) return false;
        m_ScreenYDifference += cursor->GetCursorPosition()[1] - m_LastScreenPosition[1];
        cursor->SetCursorPosition( m_LastScreenPosition );

        m_LowerThreshold = std::max<mitk::ScalarType>(0.0, m_InitialLowerThreshold - m_ScreenYDifference * m_MouseDistanceScaleFactor);
        m_UpperThreshold = std::max<mitk::ScalarType>(0.0, m_InitialUpperThreshold - m_ScreenYDifference * m_MouseDistanceScaleFactor);


        // 3. Update the contour
        mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(positionEvent->GetSender()->GetRenderWindow());
      }
    }
  }

  return true;
}

/**
If the feedback contour should be filled, then it is done here. (Contour is NOT filled, when skeletonization is done but no nice cut was found)
*/
bool mitk::FastMarchingTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) > 0.0 )
  {
    // 1. If we have a working slice, use the contour to fill a new piece on segmentation on it (or erase a piece that was selected by ipMITKSegmentationGetCutPoints)
    if ( m_WorkingSlice.IsNotNull() )
    {
      const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
      if (positionEvent)
      {
        // remember parameters for next time
        m_InitialLowerThreshold = m_LowerThreshold;
        m_InitialUpperThreshold = m_UpperThreshold;


        // 3. use contour to fill a region in our working slice
        Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
        if (feedbackContour)
        {
          Contour::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( m_WorkingSlice, feedbackContour, false, false ); // false: don't add any 0.5
          // false: don't constrain the contour to the image's inside
          if (projectedContour.IsNotNull())
          {
            FeedbackContourTool::FillContourInSlice( projectedContour, m_WorkingSlice, 1 );

            const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );

            //MITK_DEBUG << "OnMouseReleased: writing back to dimension " << affectedDimension << ", slice " << affectedSlice << " in working image" << std::endl;

            // 4. write working slice back into image volume
            this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice);
          }
        }
      }

      FeedbackContourTool::SetFeedbackContourVisible(false);
      mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

    }
  }

  m_ReferenceSlice = NULL; // don't leak
  m_WorkingSlice = NULL;


  return true;
}


bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  MITK_INFO << "AddPoint";
  return true;
}


bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  MITK_INFO << "Delete";
  return true;
}

