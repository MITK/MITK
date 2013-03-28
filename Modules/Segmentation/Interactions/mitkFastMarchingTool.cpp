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
m_MouseDistanceScaleFactor(0.5),
sigma(1.0),
alpha(-0.5),
beta(3.0)
{
  CONNECT_ACTION( AcNEWPOINT, OnMousePressed );
  CONNECT_ACTION( AcMOVEPOINT, OnMouseMoved );
  CONNECT_ACTION( AcFINISHMOVEMENT, OnMouseReleased );
  CONNECT_ACTION( AcADDPOINTRMB, OnAddPoint );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );


  m_ResultImageNode = mitk::DataNode::New();
  m_ResultImageNode->SetName("FastMarching_Preview");


  m_SliceInITK = InternalImageType::New();

  thresholder = ThresholdingFilterType::New();
  thresholder->SetLowerThreshold( 0.0 );
  thresholder->SetUpperThreshold( 100 );
  thresholder->SetOutsideValue( 0 );
  thresholder->SetInsideValue( 255 );

  smoothing = SmoothingFilterType::New();
  smoothing->SetTimeStep( 0.125 );
  smoothing->SetNumberOfIterations( 5 );
  smoothing->SetConductanceParameter( 9.0 );

  gradientMagnitude = GradientFilterType::New();
  gradientMagnitude->SetSigma( sigma );

  sigmoid = SigmoidFilterType::New();
  sigmoid->SetAlpha( alpha );
  sigmoid->SetBeta( beta );
  sigmoid->SetOutputMinimum( 0.0 );
  sigmoid->SetOutputMaximum( 1.0 );

  fastMarching = FastMarchingFilterType::New();
  fastMarching->SetStoppingValue( 50 );

  seeds = NodeContainer::New();
  seeds->Initialize();
  fastMarching->SetTrialPoints( seeds );


  //set up pipeline
  smoothing->SetInput( m_SliceInITK );
  gradientMagnitude->SetInput( smoothing->GetOutput() );
  sigmoid->SetInput( gradientMagnitude->GetOutput() );
  fastMarching->SetInput( sigmoid->GetOutput() );
  thresholder->SetInput( fastMarching->GetOutput() );
}

mitk::FastMarchingTool::~FastMarchingTool()
{
  m_ReferenceSlice = NULL; // don't leak
  m_WorkingSlice = NULL;
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
MITK_INFO << "Activated";
  Superclass::Activated();
  //m_ToolManager->GetDataStorage()->Add( this->m_ResultImageNode, this->m_ToolManager->GetReferenceData(0) );
  m_ToolManager->GetDataStorage()->Add( this->m_ResultImageNode);
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();
  //m_ToolManager->GetDataStorage()->Remove( this->m_ResultImageNode );
}


bool mitk::FastMarchingTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  if( m_LastEventSlice != m_LastEventSender->GetSlice() )
  {
    m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice( positionEvent );
    m_WorkingSlice   = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );

    CastToItkImage(m_ReferenceSlice, m_SliceInITK);
  }

  m_LastEventSlice = m_LastEventSender->GetSlice();

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



  }    else MITK_INFO << "NO SLICE";

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

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (positionEvent)
  {
    // remember parameters for next time
    m_InitialLowerThreshold = m_LowerThreshold;
    m_InitialUpperThreshold = m_UpperThreshold;

    if (dynamic_cast<mitk::Image*>(m_ResultImageNode->GetData()))
    {
      MITK_INFO << "Write result";
      //this->WriteBackSegmentationResult(positionEvent, dynamic_cast<mitk::Image*>(m_ResultImageNode->GetData()) );
    }

  }

  FeedbackContourTool::SetFeedbackContourVisible(false);
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  MITK_INFO << "AddPoint";
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  if( m_LastEventSlice != m_LastEventSender->GetSlice() )
  {
    m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice( positionEvent );

    CastToItkImage(m_ReferenceSlice, m_SliceInITK);
    smoothing->SetInput( m_SliceInITK );

    this->seeds->clear();
  }
  m_LastEventSlice = m_LastEventSender->GetSlice();

  mitk::Point3D clickInIndex;

  m_ReferenceSlice->GetGeometry()->WorldToIndex(positionEvent->GetWorldPosition(), clickInIndex);

  itk::Index<2> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );

  this->seeds->InsertElement(this->seeds->Size(), node);
  fastMarching->SetTrialPoints( seeds );

  MITK_INFO << this->seeds->size();
  MITK_INFO << clickInIndex;

  try{
    thresholder->Update();
  }
  catch( itk::ExceptionObject & excep )
  {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return true;
  }

  mitk::Image::Pointer result = mitk::Image::New();
  CastToMitkImage( thresholder->GetOutput(), result);
  result->GetGeometry()->SetOrigin(m_ReferenceSlice->GetGeometry()->GetOrigin() );
  result->GetGeometry()->SetIndexToWorldTransform(m_ReferenceSlice->GetGeometry()->GetIndexToWorldTransform() );
  m_ResultImageNode->SetData(result);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}


bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  MITK_INFO << "Delete";
  return true;
}

