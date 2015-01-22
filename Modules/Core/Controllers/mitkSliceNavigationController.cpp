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


#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkStateEvent.h"
#include "mitkCrosshairPositionEvent.h"
#include "mitkPositionEvent.h"
#include "mitkProportionalTimeGeometry.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkGlobalInteraction.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkPlaneOperation.h"
#include "mitkUndoController.h"
#include "mitkOperationEvent.h"
#include "mitkNodePredicateDataType.h"
#include "mitkStatusBar.h"
#include "mitkImage.h"

#include "mitkApplyTransformMatrixOperation.h"

#include "mitkMemoryUtilities.h"


#include <itkCommand.h>

namespace mitk {
SliceNavigationController::SliceNavigationController( const char *type )
: BaseController( type ),
  m_InputWorldGeometry3D( NULL ),
  m_InputWorldTimeGeometry( NULL ),
  m_CreatedWorldGeometry( NULL ),
  m_ViewDirection( Axial ),
  m_DefaultViewDirection( Axial ),
  m_RenderingManager( NULL ),
  m_Renderer( NULL ),
  m_Top( false ),
  m_FrontSide( false ),
  m_Rotated( false ),
  m_BlockUpdate( false ),
  m_SliceLocked( false ),
  m_SliceRotationLocked( false ),
  m_OldPos(0)
{
  typedef itk::SimpleMemberCommand< SliceNavigationController > SNCCommandType;
  SNCCommandType::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;

  sliceStepperChangedCommand = SNCCommandType::New();
  timeStepperChangedCommand = SNCCommandType::New();

  sliceStepperChangedCommand->SetCallbackFunction(
    this, &SliceNavigationController::SendSlice );

  timeStepperChangedCommand->SetCallbackFunction(
    this, &SliceNavigationController::SendTime );

  m_Slice->AddObserver( itk::ModifiedEvent(), sliceStepperChangedCommand );
  m_Time->AddObserver( itk::ModifiedEvent(), timeStepperChangedCommand );

  m_Slice->SetUnitName( "mm" );
  m_Time->SetUnitName( "ms" );

  m_Top = false;
  m_FrontSide = false;
  m_Rotated = false;
}


SliceNavigationController::~SliceNavigationController()
{
}


void
SliceNavigationController::SetInputWorldGeometry3D( const BaseGeometry *geometry )
{
  if ( geometry != NULL )
  {
    if ( const_cast< BoundingBox * >( geometry->GetBoundingBox())
         ->GetDiagonalLength2() < eps )
    {
      itkWarningMacro( "setting an empty bounding-box" );
      geometry = NULL;
    }
  }
  if ( m_InputWorldGeometry3D != geometry )
  {
    m_InputWorldGeometry3D = geometry;
    m_InputWorldTimeGeometry = NULL;
    this->Modified();
  }
}

void
SliceNavigationController::SetInputWorldTimeGeometry( const TimeGeometry *geometry )
{
  if ( geometry != NULL )
  {
    if ( const_cast< BoundingBox * >( geometry->GetBoundingBoxInWorld())
         ->GetDiagonalLength2() < eps )
    {
      itkWarningMacro( "setting an empty bounding-box" );
      geometry = NULL;
    }
  }
  if ( m_InputWorldTimeGeometry != geometry )
  {
    m_InputWorldTimeGeometry = geometry;
    m_InputWorldGeometry3D = NULL;
    this->Modified();
  }
}

RenderingManager *
SliceNavigationController::GetRenderingManager() const
{
  mitk::RenderingManager* renderingManager = m_RenderingManager.GetPointer();

  if (renderingManager != NULL)
    return renderingManager;

  if ( m_Renderer != NULL )
  {
    renderingManager = m_Renderer->GetRenderingManager();

    if (renderingManager != NULL)
      return renderingManager;
  }

  return mitk::RenderingManager::GetInstance();
}


void SliceNavigationController::SetViewDirectionToDefault()
{
  m_ViewDirection = m_DefaultViewDirection;
}

const char* SliceNavigationController::GetViewDirectionAsString()
{
    const char* viewDirectionString;
    switch(m_ViewDirection)
    {
    case SliceNavigationController::Axial:
        viewDirectionString = "Axial";
        break;

    case SliceNavigationController::Sagittal:
        viewDirectionString = "Sagittal";
        break;

    case SliceNavigationController::Frontal:
        viewDirectionString = "Coronal";
        break;

    case SliceNavigationController::Original:
        viewDirectionString = "Original";
        break;

    default:
        viewDirectionString = "No View Direction Available";
        break;
    }
    return viewDirectionString;
}

void SliceNavigationController::Update()
{
  if ( !m_BlockUpdate )
  {
    if ( m_ViewDirection == Axial )
    {
      this->Update( Axial, false, false, true );
    }
    else
    {
      this->Update( m_ViewDirection );
    }
  }
}
void
SliceNavigationController::Update(
  SliceNavigationController::ViewDirection viewDirection,
  bool top, bool frontside, bool rotated )
{
  TimeGeometry::ConstPointer worldTimeGeometry = m_InputWorldTimeGeometry;

  if( m_BlockUpdate ||
      ( m_InputWorldTimeGeometry.IsNull() && m_InputWorldGeometry3D.IsNull() ) ||
      ( (worldTimeGeometry.IsNotNull()) && (worldTimeGeometry->CountTimeSteps() == 0) )
    )
  {
    return;
  }

  m_BlockUpdate = true;

  if ( m_InputWorldTimeGeometry.IsNotNull() &&
    m_LastUpdateTime < m_InputWorldTimeGeometry->GetMTime() )
  {
    Modified();
  }
  if ( m_InputWorldGeometry3D.IsNotNull() &&
    m_LastUpdateTime < m_InputWorldGeometry3D->GetMTime() )
  {
    Modified();
  }
  this->SetViewDirection( viewDirection );
  this->SetTop( top );
  this->SetFrontSide( frontside );
  this->SetRotated( rotated );

  if ( m_LastUpdateTime < GetMTime() )
  {
    m_LastUpdateTime = GetMTime();

    // initialize the viewplane
    SlicedGeometry3D::Pointer slicedWorldGeometry = NULL;
    BaseGeometry::ConstPointer currentGeometry = NULL;
    if (m_InputWorldTimeGeometry.IsNotNull())
      if (m_InputWorldTimeGeometry->IsValidTimeStep(GetTime()->GetPos()))
        currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(GetTime()->GetPos());
      else
        currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(0);
    else
      currentGeometry = m_InputWorldGeometry3D;

    m_CreatedWorldGeometry = NULL;
    switch ( viewDirection )
    {
    case Original:
      if ( worldTimeGeometry.IsNotNull())
      {
        m_CreatedWorldGeometry = worldTimeGeometry->Clone();

        worldTimeGeometry = m_CreatedWorldGeometry.GetPointer();

        slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
          m_CreatedWorldGeometry->GetGeometryForTimeStep( this->GetTime()->GetPos() ).GetPointer() );

        if ( slicedWorldGeometry.IsNotNull() )
        {
          break;
        }
      }
      else
      {
        const SlicedGeometry3D *worldSlicedGeometry =
          dynamic_cast< const SlicedGeometry3D * >(
            currentGeometry.GetPointer());

        if ( worldSlicedGeometry != NULL )
        {
          slicedWorldGeometry = static_cast< SlicedGeometry3D * >(
            currentGeometry->Clone().GetPointer());
          break;
        }
      }
      //else: use Axial: no "break" here!!

    case Axial:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(
        currentGeometry, PlaneGeometry::Axial,
        top, frontside, rotated );
      slicedWorldGeometry->SetSliceNavigationController( this );
      break;

    case Frontal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( currentGeometry,
        PlaneGeometry::Frontal, top, frontside, rotated );
      slicedWorldGeometry->SetSliceNavigationController( this );
      break;

    case Sagittal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( currentGeometry,
        PlaneGeometry::Sagittal, top, frontside, rotated );
      slicedWorldGeometry->SetSliceNavigationController( this );
      break;
    default:
      itkExceptionMacro("unknown ViewDirection");
    }

    m_Slice->SetPos( 0 );
    m_Slice->SetSteps( (int)slicedWorldGeometry->GetSlices() );

    if ( m_CreatedWorldGeometry.IsNull() )
    {
      // initialize TimeGeometry
      m_CreatedWorldGeometry = ProportionalTimeGeometry::New();
    }
    if ( worldTimeGeometry.IsNull())
    {
      m_CreatedWorldGeometry = ProportionalTimeGeometry::New();
      dynamic_cast<ProportionalTimeGeometry *>(m_CreatedWorldGeometry.GetPointer())->Initialize(slicedWorldGeometry, 1);
      m_Time->SetSteps( 0 );
      m_Time->SetPos( 0 );
      m_Time->InvalidateRange();
    }
    else
    {
      m_BlockUpdate = true;
      m_Time->SetSteps( worldTimeGeometry->CountTimeSteps() );
      m_Time->SetPos( 0 );

      const TimeBounds &timeBounds = worldTimeGeometry->GetTimeBounds();
      m_Time->SetRange( timeBounds[0], timeBounds[1] );

      m_BlockUpdate = false;

      assert( worldTimeGeometry->GetGeometryForTimeStep( this->GetTime()->GetPos() ).IsNotNull() );

      TimePointType minimumTimePoint = worldTimeGeometry->TimeStepToTimePoint(this->GetTime()->GetPos());
      TimePointType stepDuration = worldTimeGeometry->TimeStepToTimePoint(this->GetTime()->GetPos()+1)-worldTimeGeometry->TimeStepToTimePoint(this->GetTime()->GetPos());

      //@todo implement for non-evenly-timed geometry!
      m_CreatedWorldGeometry = ProportionalTimeGeometry::New();
      dynamic_cast<ProportionalTimeGeometry *>(m_CreatedWorldGeometry.GetPointer())->Initialize(slicedWorldGeometry, worldTimeGeometry->CountTimeSteps());
      dynamic_cast<ProportionalTimeGeometry *>(m_CreatedWorldGeometry.GetPointer())->GetMinimumTimePoint(minimumTimePoint);
      dynamic_cast<ProportionalTimeGeometry *>(m_CreatedWorldGeometry.GetPointer())->SetStepDuration(stepDuration);
    }
  }

  // unblock update; we may do this now, because if m_BlockUpdate was already
  // true before this method was entered, then we will never come here.
  m_BlockUpdate = false;

  // Send the geometry. Do this even if nothing was changed, because maybe
  // Update() was only called to re-send the old geometry and time/slice data.
  this->SendCreatedWorldGeometry();
  this->SendSlice();
  this->SendTime();

  // Adjust the stepper range of slice stepper according to geometry
  this->AdjustSliceStepperRange();
}

void
SliceNavigationController::SendCreatedWorldGeometry()
{
  // Send the geometry. Do this even if nothing was changed, because maybe
  // Update() was only called to re-send the old geometry.
  if ( !m_BlockUpdate )
  {
    this->InvokeEvent( GeometrySendEvent(m_CreatedWorldGeometry, 0) );
  }
}

void
SliceNavigationController::SendCreatedWorldGeometryUpdate()
{
  if ( !m_BlockUpdate )
  {
    this->InvokeEvent(
      GeometryUpdateEvent(m_CreatedWorldGeometry, m_Slice->GetPos()) );
  }
}

void
SliceNavigationController::SendSlice()
{
  if ( !m_BlockUpdate )
  {
    if ( m_CreatedWorldGeometry.IsNotNull() )
    {
      this->InvokeEvent(
        GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()) );

      // send crosshair event
      crosshairPositionEvent.Send();

      // Request rendering update for all views
      this->GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void
SliceNavigationController::SendTime()
{
  if ( !m_BlockUpdate )
  {
    if ( m_CreatedWorldGeometry.IsNotNull() )
    {
      this->InvokeEvent(
        GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()) );

      // Request rendering update for all views
      this->GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void
SliceNavigationController::SetGeometry( const itk::EventObject & )
{
}

void
SliceNavigationController
::SetGeometryTime( const itk::EventObject &geometryTimeEvent )
{
  if (m_CreatedWorldGeometry.IsNull())
  {
    return;
  }

  const SliceNavigationController::GeometryTimeEvent *timeEvent =
    dynamic_cast< const SliceNavigationController::GeometryTimeEvent * >(
      &geometryTimeEvent);

  assert( timeEvent != NULL );

  TimeGeometry *timeGeometry = timeEvent->GetTimeGeometry();
  assert( timeGeometry != NULL );

  int timeStep = (int) timeEvent->GetPos();
  ScalarType timeInMS;
  timeInMS = timeGeometry->TimeStepToTimePoint( timeStep );
  timeStep = m_CreatedWorldGeometry->TimePointToTimeStep( timeInMS );
  this->GetTime()->SetPos( timeStep );
}

void
SliceNavigationController
::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(
      &geometrySliceEvent);
  assert(sliceEvent!=NULL);

  this->GetSlice()->SetPos(sliceEvent->GetPos());
}

void
SliceNavigationController::SelectSliceByPoint( const Point3D &point )
{
  if (m_CreatedWorldGeometry.IsNull())
  {
    return;
  }

  //@todo add time to PositionEvent and use here!!
  SlicedGeometry3D* slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
    m_CreatedWorldGeometry->GetGeometryForTimeStep( this->GetTime()->GetPos() ).GetPointer() );

  if ( slicedWorldGeometry )
  {
    int bestSlice = -1;
    double bestDistance = itk::NumericTraits<double>::max();

    int s, slices;
    slices = slicedWorldGeometry->GetSlices();
    if ( slicedWorldGeometry->GetEvenlySpaced() )
    {
      mitk::PlaneGeometry *plane = slicedWorldGeometry->GetPlaneGeometry( 0 );

      const Vector3D &direction = slicedWorldGeometry->GetDirectionVector();

      Point3D projectedPoint;
      plane->Project( point, projectedPoint );

      // Check whether the point is somewhere within the slice stack volume;
      // otherwise, the defualt slice (0) will be selected
      if ( direction[0] * (point[0] - projectedPoint[0])
         + direction[1] * (point[1] - projectedPoint[1])
         + direction[2] * (point[2] - projectedPoint[2]) >= 0 )
      {
        bestSlice = (int)(plane->Distance( point )
          / slicedWorldGeometry->GetSpacing()[2] + 0.5);
      }
    }
    else
    {
      Point3D projectedPoint;
      for ( s = 0; s < slices; ++s )
      {
        slicedWorldGeometry->GetPlaneGeometry( s )->Project( point, projectedPoint );
        Vector3D distance = projectedPoint - point;
        ScalarType currentDistance = distance.GetSquaredNorm();

        if ( currentDistance < bestDistance )
        {
          bestDistance = currentDistance;
          bestSlice = s;
        }
      }
    }
    if ( bestSlice >= 0 )
    {
      this->GetSlice()->SetPos( bestSlice );
    }
    else
    {
      this->GetSlice()->SetPos( 0 );
    }
    this->SendCreatedWorldGeometryUpdate();
  }
}


void
SliceNavigationController::ReorientSlices( const Point3D &point,
  const Vector3D &normal )
{
  if (m_CreatedWorldGeometry.IsNull())
  {
    return;
  }

  PlaneOperation op( OpORIENT, point, normal );

  m_CreatedWorldGeometry->ExecuteOperation( &op );

  this->SendCreatedWorldGeometryUpdate();
}

void SliceNavigationController::ReorientSlices(const mitk::Point3D &point,
   const mitk::Vector3D &axisVec0, const mitk::Vector3D &axisVec1 )
{
   if (m_CreatedWorldGeometry)
   {
     PlaneOperation op( OpORIENT, point, axisVec0, axisVec1 );
     m_CreatedWorldGeometry->ExecuteOperation( &op );

     this->SendCreatedWorldGeometryUpdate();
   }
}

mitk::TimeGeometry *
SliceNavigationController::GetCreatedWorldGeometry()
{
  return m_CreatedWorldGeometry;
}

const mitk::BaseGeometry *
SliceNavigationController::GetCurrentGeometry3D()
{
  if ( m_CreatedWorldGeometry.IsNotNull() )
  {
    return m_CreatedWorldGeometry->GetGeometryForTimeStep( this->GetTime()->GetPos() );
  }
  else
  {
    return NULL;
  }
}


const mitk::PlaneGeometry *
SliceNavigationController::GetCurrentPlaneGeometry()
{
  const mitk::SlicedGeometry3D *slicedGeometry =
    dynamic_cast< const mitk::SlicedGeometry3D * >
      ( this->GetCurrentGeometry3D() );

  if ( slicedGeometry )
  {
    const mitk::PlaneGeometry *planeGeometry =
        ( slicedGeometry->GetPlaneGeometry(this->GetSlice()->GetPos()) );
    return planeGeometry;
  }
  else
  {
    return NULL;
  }
}


void
SliceNavigationController::SetRenderer( BaseRenderer *renderer )
{
  m_Renderer = renderer;
}

BaseRenderer *
SliceNavigationController::GetRenderer() const
{
  return m_Renderer;
}



void
SliceNavigationController::AdjustSliceStepperRange()
{
  const mitk::SlicedGeometry3D *slicedGeometry =
    dynamic_cast< const mitk::SlicedGeometry3D * >
      ( this->GetCurrentGeometry3D() );

  const Vector3D &direction = slicedGeometry->GetDirectionVector();

  int c = 0;
  int i, k = 0;
  for ( i = 0; i < 3; ++i )
  {
    if ( fabs(direction[i]) < 0.000000001 ) { ++c; }
    else { k = i; }
  }

  if ( c == 2 )
  {
    ScalarType min = slicedGeometry->GetOrigin()[k];
    ScalarType max = min + slicedGeometry->GetExtentInMM( k );

    m_Slice->SetRange( min, max );
  }
  else
  {
    m_Slice->InvalidateRange();
  }
}


void
SliceNavigationController::ExecuteOperation( Operation *operation )
{
  // switch on type
  // - select best slice for a given point
  // - rotate created world geometry according to Operation->SomeInfo()
  if ( !operation || m_CreatedWorldGeometry.IsNull())
  {
    return;
  }

  switch ( operation->GetOperationType() )
  {
    case OpMOVE: // should be a point operation
    {
      if ( !m_SliceLocked ) //do not move the cross position
      {
        // select a slice
        PointOperation *po = dynamic_cast< PointOperation * >( operation );
        if ( po && po->GetIndex() == -1 )
        {
          this->SelectSliceByPoint( po->GetPoint() );
        }
        else if ( po && po->GetIndex() != -1 ) // undo case because index != -1, index holds the old position of this slice
        {
          this->GetSlice()->SetPos( po->GetIndex() );
        }
      }
      break;
    }
    case OpRESTOREPLANEPOSITION:
      {
        m_CreatedWorldGeometry->ExecuteOperation( operation );

        this->SendCreatedWorldGeometryUpdate();

        break;
      }
    case OpAPPLYTRANSFORMMATRIX:
      {
        m_CreatedWorldGeometry->ExecuteOperation( operation );

        this->SendCreatedWorldGeometryUpdate();

        break;
      }
    default:
    {
      // do nothing
      break;
    }
  }
}

mitk::DataNode::Pointer SliceNavigationController::GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes,mitk::Point3D worldposition)
{
  mitk::DataNode::Pointer node;
  int  maxlayer = -32768;
  bool isHelper (false);
  if(nodes.IsNotNull())
  {
    for (unsigned int x = 0; x < nodes->size(); x++)
    {
      nodes->at(x)->GetBoolProperty("helper object", isHelper);
      if(nodes->at(x)->GetData()->GetGeometry()->IsInside(worldposition) && isHelper == false)
      {
        int layer = 0;
        if(!(nodes->at(x)->GetIntProperty("layer", layer))) continue;
        if(layer > maxlayer)
        {
          if(static_cast<mitk::DataNode::Pointer>(nodes->at(x))->IsVisible(m_Renderer))
          {
            node = nodes->at(x);
            maxlayer = layer;
          }
        }
      }
    }
  }
  return node;
}
// Relict from the old times, when automous decisions were accepted
// behavior. Remains in here, because some RenderWindows do exist outside
// of StdMultiWidgets.
bool
SliceNavigationController
::ExecuteAction( Action* action, StateEvent const* stateEvent )
{
  bool ok = false;

  const PositionEvent* posEvent = dynamic_cast< const PositionEvent * >(
    stateEvent->GetEvent() );
  if ( posEvent != NULL )
  {
    if ( m_CreatedWorldGeometry.IsNull() )
    {
      return true;
    }
    switch (action->GetActionId())
    {
    case AcMOVE:
      {
        BaseRenderer *baseRenderer = posEvent->GetSender();
        if ( !baseRenderer )
        {
          baseRenderer = const_cast<BaseRenderer *>(
            GlobalInteraction::GetInstance()->GetFocus() );
        }
        if ( baseRenderer )
          if ( baseRenderer->GetMapperID() == 1 )
          {
            PointOperation doOp(OpMOVE, posEvent->GetWorldPosition());

            this->ExecuteOperation( &doOp );

            // If click was performed in this render window than we have to update the status bar information about position and pixel value.
            if(baseRenderer == m_Renderer)
            {
              {
                std::string statusText;
                TNodePredicateDataType<mitk::Image>::Pointer isImageData = TNodePredicateDataType<mitk::Image>::New();

                mitk::DataStorage::SetOfObjects::ConstPointer nodes = baseRenderer->GetDataStorage()->GetSubset(isImageData).GetPointer();
                mitk::Point3D worldposition = posEvent->GetWorldPosition();
                //int  maxlayer = -32768;
                mitk::Image::Pointer image3D;
                mitk::DataNode::Pointer node;
                mitk::DataNode::Pointer topSourceNode;

                bool isBinary (false);
                int component = 0;

                node = this->GetTopLayerNode(nodes,worldposition);
                if(node.IsNotNull())
                {
                  node->GetBoolProperty("binary", isBinary);
                  if(isBinary)
                  {
                    mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = baseRenderer->GetDataStorage()->GetSources(node, NULL, true);
                    if(!sourcenodes->empty())
                    {
                      topSourceNode = this->GetTopLayerNode(sourcenodes,worldposition);
                    }
                    if(topSourceNode.IsNotNull())
                    {
                      image3D = dynamic_cast<mitk::Image*>(topSourceNode->GetData());
                      topSourceNode->GetIntProperty("Image.Displayed Component", component);
                    }
                    else
                    {
                      image3D = dynamic_cast<mitk::Image*>(node->GetData());
                      node->GetIntProperty("Image.Displayed Component", component);
                    }
                  }
                  else
                  {
                    image3D = dynamic_cast<mitk::Image*>(node->GetData());
                    node->GetIntProperty("Image.Displayed Component", component);
                  }
                }
                std::stringstream stream;
                stream.imbue(std::locale::classic());

                // get the position and gray value from the image and build up status bar text
                if(image3D.IsNotNull())
                {
                  itk::Index<3> p;
                  image3D->GetGeometry()->WorldToIndex(worldposition, p);
                  stream.precision(2);
                  stream<<"Position: <" << std::fixed <<worldposition[0] << ", " << std::fixed << worldposition[1] << ", " << std::fixed << worldposition[2] << "> mm";
                  stream<<"; Index: <"<<p[0] << ", " << p[1] << ", " << p[2] << "> ";
                  mitk::ScalarType pixelValue = image3D->GetPixelValueByIndex(p, baseRenderer->GetTimeStep(), component);

                  if (fabs(pixelValue)>1000000 || fabs(pixelValue) < 0.01)
                  {
                    stream<<"; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: " << std::scientific<< pixelValue <<"  ";
                  }
                  else
                  {
                    stream<<"; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: "<< pixelValue <<"  ";
                  }
                }
                else
                {
                  stream << "No image information at this position!";
                }

                statusText = stream.str();
                mitk::StatusBar::GetInstance()->DisplayGreyValueText(statusText.c_str());
              }
            }
            ok = true;
            break;
          }
      }
    default:
      ok = true;
      break;
    }
    return ok;
  }

  const DisplayPositionEvent *displPosEvent =
    dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );

  if ( displPosEvent != NULL )
  {
    return true;
  }

  return false;
}
} // namespace
