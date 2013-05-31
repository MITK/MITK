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

#include "itkOrImageFilter.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool, "FastMarching tool");
}


mitk::FastMarchingTool::FastMarchingTool()
:FeedbackContourTool("PressMoveReleaseAndPointSetting"),
m_IsLivePreviewEnabled(false),
m_LowerThreshold(0.0),
m_UpperThreshold(200.0),
m_StoppingValue(100),
sigma(1.0),
alpha(-0.5),
beta(3.0)
{
  CONNECT_ACTION( AcADDPOINTRMB, OnAddPoint );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );

  m_SliceInITK = InternalImageType::New();

  thresholder = ThresholdingFilterType::New();
  thresholder->SetLowerThreshold( m_LowerThreshold );
  thresholder->SetUpperThreshold( m_UpperThreshold );
  thresholder->SetOutsideValue( 0 );
  thresholder->SetInsideValue( 1.0 );

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
  fastMarching->SetStoppingValue( m_StoppingValue );

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
  m_ReferenceSlice = NULL;
  m_WorkingSlice = NULL;
  m_PositionEvent = NULL;
}


float mitk::FastMarchingTool::CanHandleEvent( StateEvent const *stateEvent) const
{
  float returnValue = Superclass::CanHandleEvent(stateEvent);

  //we can handle delete
  if(stateEvent->GetId() == 12 )
  {
    returnValue = 1.0;
  }

  return returnValue;
}


const char** mitk::FastMarchingTool::GetXPM() const
{
  return NULL;// mitkFastMarchingTool_xpm;
}

std::string mitk::FastMarchingTool::GetIconPath() const
{
  return ":/Segmentation/FastMarching_48x48.png";
}


const char* mitk::FastMarchingTool::GetName() const
{
  return "FastMarching";
}

void mitk::FastMarchingTool::SetUpperThreshold(float value)
{
  m_UpperThreshold = value;
  thresholder->SetUpperThreshold( m_UpperThreshold );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::SetLowerThreshold(float value)
{
  m_LowerThreshold = value;
  thresholder->SetLowerThreshold( m_LowerThreshold );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::SetMu(float value)
{
  beta = value;
  sigmoid->SetBeta( beta );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::SetStandardDeviation(float value)
{
  alpha = value;
  sigmoid->SetAlpha( alpha );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::SetStoppingValue(float value)
{
  m_StoppingValue = value;
  fastMarching->SetStoppingValue( m_StoppingValue );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::SetLivePreviewEnabled(bool enabled)
{
  m_IsLivePreviewEnabled = enabled;

  if(enabled)//an update may be needed
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool::Activated()
{
  Superclass::Activated();

  m_ResultImageNode = mitk::DataNode::New();
  m_ResultImageNode->SetName("FastMarching_Preview");
  m_ResultImageNode->SetBoolProperty("helper object", true);
  m_ResultImageNode->SetColor(0.0, 1.0, 0.0);
  m_ResultImageNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( this->m_ResultImageNode);

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("Seeds_Preview");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( this->m_SeedsAsPointSetNode);
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();
  m_ToolManager->GetDataStorage()->Remove( this->m_ResultImageNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  m_ResultImageNode = NULL;
}





void mitk::FastMarchingTool::ConfirmSegmentation()
{
  /*+++++++combine preview image with already performed segmentation++++++*/
  if (m_PositionEvent)
  {
    // remember parameters for next time
    if (dynamic_cast<mitk::Image*>(m_ResultImageNode->GetData()))
    {
      //logical or combination of preview and segmentation slice
      OutputImageType::Pointer segmentationSlice = OutputImageType::New();

      mitk::Image::Pointer workingSlice = GetAffectedWorkingSlice( m_PositionEvent );
      CastToItkImage( workingSlice, segmentationSlice );

      typedef itk::OrImageFilter<OutputImageType, OutputImageType> OrImageFilterType;
      OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

      orFilter->SetInput(0, thresholder->GetOutput());
      orFilter->SetInput(1, segmentationSlice);
      orFilter->Update();

      mitk::Image::Pointer segmentationResult = mitk::Image::New();

      mitk::CastToMitkImage(orFilter->GetOutput(), segmentationResult);
      segmentationResult->GetGeometry()->SetOrigin(workingSlice->GetGeometry()->GetOrigin());
      segmentationResult->GetGeometry()->SetIndexToWorldTransform(workingSlice->GetGeometry()->GetIndexToWorldTransform());

      //write to segmentation volume and hide preview image
      this->WriteBackSegmentationResult(m_PositionEvent, segmentationResult );
      this->m_ResultImageNode->SetVisibility(false);
      this->ClearSeeds();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdate( m_PositionEvent->GetSender()->GetRenderWindow() );
  }
}


bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  /*++++++++++++Add a new seed point for FastMarching algorithm+++++++++++*/
  const mitk::PositionEvent* p = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!p) return false;

  m_PositionEvent = new PositionEvent(p->GetSender(), p->GetType(), p->GetButton(), p->GetButtonState(), p->GetKey(), p->GetDisplayPosition(), p->GetWorldPosition());

  //if click happpened in another renderwindow or slice then reset pipeline and preview
  if( (m_LastEventSender != m_PositionEvent->GetSender()) || (m_LastEventSlice != m_PositionEvent->GetSender()->GetSlice()) )
  {
    this->ResetFastMarching(m_PositionEvent);
  }
  m_LastEventSender = m_PositionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();


  mitk::Point3D clickInIndex;

  m_ReferenceSlice->GetGeometry()->WorldToIndex(m_PositionEvent->GetWorldPosition(), clickInIndex);

  itk::Index<2> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );

  this->seeds->InsertElement(this->seeds->Size(), node);
  fastMarching->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), m_PositionEvent->GetWorldPosition());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //preview result by updating the pipeline - this is not done automatically for changing seeds
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
  return true;
}


bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  /*++++++++++delete last seed point++++++++*/
  if(!(this->seeds->empty()))
  {
    //delete last element of seeds container
    this->seeds->pop_back();
    fastMarching->Modified();

    //delete last point in pointset - somehow ugly
    m_SeedsAsPointSet->GetPointSet()->GetPoints()->DeleteIndex(m_SeedsAsPointSet->GetSize() - 1);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    //preview result by updating the pipeline - this is not done automatically for changing seeds
    if(m_IsLivePreviewEnabled)
      this->UpdatePreviewImage();
  }
  return true;
}


void mitk::FastMarchingTool::UpdatePreviewImage()
{
  /*++++++++update FastMarching pipeline and show result++++++++*/
  if(m_ReferenceSlice.IsNotNull())
  {
    try{
      thresholder->UpdateLargestPossibleRegion();
    }
    catch( itk::ExceptionObject & excep )
    {
      std::cerr << "Exception caught !" << std::endl;
      std::cerr << excep << std::endl;
      std::cerr << "Reseting step." << std::endl;
      this->ResetToInitialState();
      return;
    }

    //make output visible
    mitk::Image::Pointer result = mitk::Image::New();
    CastToMitkImage( thresholder->GetOutput(), result);
    result->GetGeometry()->SetOrigin(m_ReferenceSlice->GetGeometry()->GetOrigin() );
    result->GetGeometry()->SetIndexToWorldTransform(m_ReferenceSlice->GetGeometry()->GetIndexToWorldTransform() );
    m_ResultImageNode->SetData(result);
    m_ResultImageNode->SetVisibility(true);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void mitk::FastMarchingTool::ClearSeeds()
{
  /*++++++++clear seeds for FastMarching as well as the PointSet for visualization++++++++*/
  this->seeds->clear();
  fastMarching->Modified();

  m_SeedsAsPointSet->Clear();

  m_ResultImageNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void mitk::FastMarchingTool::ResetFastMarching(const PositionEvent* positionEvent)
{
  /*++++++++reset all relevant inputs for FastMarching++++++++*/
  //reset reference slice according to the plane where the click happened
  m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice( positionEvent );

  //reset input of FastMarching pipeline
  CastToItkImage(m_ReferenceSlice, m_SliceInITK);
  smoothing->SetInput( m_SliceInITK );

  //clear all seeds and preview empty result
  this->ClearSeeds();
  this->UpdatePreviewImage();
}


void mitk::FastMarchingTool::ResetToInitialState()
{
  m_SeedsAsPointSet->Clear();
  m_ResultImageNode->SetVisibility(false);

  m_SliceInITK = InternalImageType::New();

  thresholder = ThresholdingFilterType::New();
  thresholder->SetLowerThreshold( m_LowerThreshold );
  thresholder->SetUpperThreshold( m_UpperThreshold );
  thresholder->SetOutsideValue( 0 );
  thresholder->SetInsideValue( 1.0 );

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
  fastMarching->SetStoppingValue( m_StoppingValue );

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
