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

#include "mitkFastMarchingTool3D.h"
#include "mitkToolManager.h"


#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkApplicationCursor.h"

//#include "mitkFastMarchingTool3D.xpm"

#include "mitkInteractionConst.h"

#include "itkOrImageFilter.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool3D, "FastMarching tool");
}


mitk::FastMarchingTool3D::FastMarchingTool3D()
:FeedbackContourTool("PressMoveReleaseAndPointSetting"),
m_IsLivePreviewEnabled(false),
m_LowerThreshold(200),
m_UpperThreshold(200),
m_InitialLowerThreshold(0.0),
m_InitialUpperThreshold(100.0),
m_InitialStoppingValue(100),
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
  thresholder->SetLowerThreshold( m_InitialLowerThreshold );
  thresholder->SetUpperThreshold( m_InitialUpperThreshold );
  thresholder->SetOutsideValue( 0 );
  thresholder->SetInsideValue( 1.0 );

  smoothing = SmoothingFilterType::New();
  smoothing->SetTimeStep( 0.05 );
  smoothing->SetNumberOfIterations( 2 );
  smoothing->SetConductanceParameter( 9.0 );

  gradientMagnitude = GradientFilterType::New();
  gradientMagnitude->SetSigma( sigma );

  sigmoid = SigmoidFilterType::New();
  sigmoid->SetAlpha( alpha );
  sigmoid->SetBeta( beta );
  sigmoid->SetOutputMinimum( 0.0 );
  sigmoid->SetOutputMaximum( 1.0 );

  fastMarching = FastMarchingFilterType::New();
  fastMarching->SetStoppingValue( m_InitialStoppingValue );

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

mitk::FastMarchingTool3D::~FastMarchingTool3D()
{
  m_ReferenceSlice = NULL;
  m_WorkingSlice = NULL;
}


float mitk::FastMarchingTool3D::CanHandleEvent( StateEvent const *stateEvent) const
{
  float returnValue = Superclass::CanHandleEvent(stateEvent);

  //we can handle delete
  if(stateEvent->GetId() == 12 )
  {
    returnValue = 1.0;
  }

  return returnValue;
}


const char** mitk::FastMarchingTool3D::GetXPM() const
{
  return NULL;//mitkFastMarchingTool3D_xpm;
}

std::string mitk::FastMarchingTool3D::GetIconPath() const
{
  return ":/Segmentation/FastMarching_48x48.png";
}

const char* mitk::FastMarchingTool3D::GetName() const
{
  return "FastMarching3D";
}

void mitk::FastMarchingTool3D::SetUpperThreshold(int value)
{
  m_UpperThreshold = (float)value / 10.0;
  thresholder->SetUpperThreshold( m_UpperThreshold );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool3D::SetLowerThreshold(int value)
{
  m_LowerThreshold = (float)value / 10.0;
  thresholder->SetLowerThreshold( m_LowerThreshold );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool3D::SetMu(int value)
{
  beta = (float)value / 10.0;
  sigmoid->SetBeta( beta );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool3D::SetStandardDeviation(int value)
{
  alpha = (float)value / 10.0;
  sigmoid->SetAlpha( alpha );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool3D::SetStoppingValue(int value)
{
  m_StoppingValue = (float)value / 10.0;
  fastMarching->SetStoppingValue( m_StoppingValue );
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
}

void mitk::FastMarchingTool3D::SetLivePreviewEnabled(bool enabled)
{
  m_IsLivePreviewEnabled = enabled;
}

void mitk::FastMarchingTool3D::Activated()
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

  m_ReferenceSlice = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());
  CastToItkImage(m_ReferenceSlice, m_SliceInITK);
  smoothing->SetInput( m_SliceInITK );

}

void mitk::FastMarchingTool3D::Deactivated()
{
  Superclass::Deactivated();
  m_ToolManager->GetDataStorage()->Remove( this->m_ResultImageNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  this->ClearSeeds();
  m_ResultImageNode = NULL;
}


void mitk::FastMarchingTool3D::ConfirmSegmentation()
{
  //if not preview was not enabled the pipeline may be not executed yet
  if(!m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();

  /*+++++++combine preview image with already performed segmentation++++++*/
  if (dynamic_cast<mitk::Image*>(m_ResultImageNode->GetData()))
  {
    //logical or combination of preview and segmentation slice
    OutputImageType::Pointer segmentationImage = OutputImageType::New();

    mitk::Image::Pointer workingImage = dynamic_cast<mitk::Image*>(this->m_ToolManager->GetWorkingData(0)->GetData());
    CastToItkImage( workingImage, segmentationImage );

    typedef itk::OrImageFilter<OutputImageType, OutputImageType> OrImageFilterType;
    OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

    orFilter->SetInput(0, thresholder->GetOutput());
    orFilter->SetInput(1, segmentationImage);
    orFilter->Update();

    mitk::Image::Pointer segmentationResult = mitk::Image::New();

    mitk::CastToMitkImage(orFilter->GetOutput(), segmentationResult);
    segmentationResult->GetGeometry()->SetOrigin(workingImage->GetGeometry()->GetOrigin());
    segmentationResult->GetGeometry()->SetIndexToWorldTransform(workingImage->GetGeometry()->GetIndexToWorldTransform());

    this->m_ResultImageNode->SetData(segmentationResult);
    this->m_ToolManager->GetWorkingData(0)->SetData(segmentationResult);

    this->m_ResultImageNode->SetVisibility(false);
    this->ClearSeeds();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


bool mitk::FastMarchingTool3D::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  /*++++++++++++Add a new seed point for FastMarching algorithm+++++++++++*/
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::Point3D clickInIndex;

  m_ReferenceSlice->GetGeometry()->WorldToIndex(positionEvent->GetWorldPosition(), clickInIndex);
  itk::Index<3> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];
  seedPosition[2] = clickInIndex[2];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  this->seeds->InsertElement(this->seeds->Size(), node);
  fastMarching->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), positionEvent->GetWorldPosition());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //preview result by updating the pipeline - this is not done automatically for changing seeds
  if(m_IsLivePreviewEnabled)
    this->UpdatePreviewImage();
  return true;
}


bool mitk::FastMarchingTool3D::OnDelete(Action* action, const StateEvent* stateEvent)
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


void mitk::FastMarchingTool3D::UpdatePreviewImage()
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


void mitk::FastMarchingTool3D::ClearSeeds()
{
  /*++++++++clear seeds for FastMarching as well as the PointSet for visualization++++++++*/
  this->seeds->clear();
  fastMarching->Modified();

  m_SeedsAsPointSet->Clear();
}


void mitk::FastMarchingTool3D::ResetFastMarching(const PositionEvent* positionEvent)
{
  /*++++++++reset all relevant inputs for FastMarching++++++++*/
  //reset reference slice according to the plane where the click happened
  m_ReferenceSlice = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());

  //reset input of FastMarching pipeline
  CastToItkImage(m_ReferenceSlice, m_SliceInITK);
  smoothing->SetInput( m_SliceInITK );

  //clear all seeds and preview empty result
  this->ClearSeeds();
  this->UpdatePreviewImage();
}
