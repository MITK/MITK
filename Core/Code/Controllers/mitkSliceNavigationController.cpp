/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"
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
#include <mitkIpPicTypeMultiplex.h>

#include "mitkMemoryUtilities.h"


#include <itkCommand.h>

namespace mitk {

SliceNavigationController::SliceNavigationController( const char *type ) 
: BaseController( type ),
  m_InputWorldGeometry( NULL ),
  m_CreatedWorldGeometry( NULL ), 
  m_ViewDirection( Transversal ),
  m_DefaultViewDirection( Transversal ),
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
SliceNavigationController::SetInputWorldGeometry( const Geometry3D *geometry )
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
  if ( m_InputWorldGeometry != geometry )
  {
    m_InputWorldGeometry = geometry;
    this->Modified();
  }
}

RenderingManager *
SliceNavigationController::GetRenderingManager() const
{
  mitk::RenderingManager* renderingManager = m_RenderingManager.GetPointer();
  if(renderingManager == NULL)
    return mitk::RenderingManager::GetInstance();
  return renderingManager;
}


void SliceNavigationController::SetViewDirectionToDefault()
{
  m_ViewDirection = m_DefaultViewDirection;
}

void SliceNavigationController::Update()
{
  if ( !m_BlockUpdate )
  {
    if ( m_ViewDirection == Transversal )
    {
      this->Update( Transversal, false, false, true );
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
  const TimeSlicedGeometry* worldTimeSlicedGeometry =
    dynamic_cast< const TimeSlicedGeometry * >(
      m_InputWorldGeometry.GetPointer() );

  if( m_BlockUpdate || 
      m_InputWorldGeometry.IsNull() || 
      ( (worldTimeSlicedGeometry != NULL) && (worldTimeSlicedGeometry->GetTimeSteps() == 0) )
    )
  {
    return;
  }

  m_BlockUpdate = true;

  if ( m_LastUpdateTime < m_InputWorldGeometry->GetMTime() )
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
    
    m_CreatedWorldGeometry = NULL;
    switch ( viewDirection )
    {
    case Original:
      if ( worldTimeSlicedGeometry != NULL )
      {
        m_CreatedWorldGeometry = static_cast< TimeSlicedGeometry * >(
          m_InputWorldGeometry->Clone().GetPointer() );

        worldTimeSlicedGeometry = m_CreatedWorldGeometry.GetPointer();

        slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
          m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos() ) );

        if ( slicedWorldGeometry.IsNotNull() )
        {
          break;
        }
      }
      else
      {
        const SlicedGeometry3D *worldSlicedGeometry =
          dynamic_cast< const SlicedGeometry3D * >(
            m_InputWorldGeometry.GetPointer());

        if ( worldSlicedGeometry != NULL )
        {
          slicedWorldGeometry = static_cast< SlicedGeometry3D * >(
            m_InputWorldGeometry->Clone().GetPointer());
          break;
        }
      }
      //else: use Transversal: no "break" here!!

    case Transversal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(
        m_InputWorldGeometry, PlaneGeometry::Transversal,
        top, frontside, rotated );
      slicedWorldGeometry->SetSliceNavigationController( this );
      break;

    case Frontal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( m_InputWorldGeometry,
        PlaneGeometry::Frontal, top, frontside, rotated );
      slicedWorldGeometry->SetSliceNavigationController( this );
      break;

    case Sagittal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( m_InputWorldGeometry,
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
      // initialize TimeSlicedGeometry
      m_CreatedWorldGeometry = TimeSlicedGeometry::New();
    }
    if ( worldTimeSlicedGeometry == NULL )
    {
      m_CreatedWorldGeometry->InitializeEvenlyTimed( slicedWorldGeometry, 1 );
      m_Time->SetSteps( 0 );
      m_Time->SetPos( 0 );
      m_Time->InvalidateRange();
    }
    else
    {
      m_BlockUpdate = true;
      m_Time->SetSteps( worldTimeSlicedGeometry->GetTimeSteps() );
      m_Time->SetPos( 0 );

      const TimeBounds &timeBounds = worldTimeSlicedGeometry->GetTimeBounds();
      m_Time->SetRange( timeBounds[0], timeBounds[1] );

      m_BlockUpdate = false;

      assert( worldTimeSlicedGeometry->GetGeometry3D( this->GetTime()->GetPos() ) != NULL );

      slicedWorldGeometry->SetTimeBounds(
        worldTimeSlicedGeometry->GetGeometry3D( this->GetTime()->GetPos() )->GetTimeBounds() );

      //@todo implement for non-evenly-timed geometry!
      m_CreatedWorldGeometry->InitializeEvenlyTimed(
        slicedWorldGeometry, worldTimeSlicedGeometry->GetTimeSteps() );
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
  const SliceNavigationController::GeometryTimeEvent *timeEvent =
    dynamic_cast< const SliceNavigationController::GeometryTimeEvent * >(
      &geometryTimeEvent);

  assert( timeEvent != NULL );

  TimeSlicedGeometry *timeSlicedGeometry = timeEvent->GetTimeSlicedGeometry();
  assert( timeSlicedGeometry != NULL );

  if ( m_CreatedWorldGeometry.IsNotNull() )
  {
    int timeStep = (int) timeEvent->GetPos();
    ScalarType timeInMS;
    timeInMS = timeSlicedGeometry->TimeStepToMS( timeStep );
    timeStep = m_CreatedWorldGeometry->MSToTimeStep( timeInMS );
    this->GetTime()->SetPos( timeStep );
  }
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
  //@todo add time to PositionEvent and use here!!
  SlicedGeometry3D* slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
    m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos() ) );

  if ( slicedWorldGeometry )
  {
    int bestSlice = -1;
    double bestDistance = itk::NumericTraits<double>::max();

    int s, slices;
    slices = slicedWorldGeometry->GetSlices();
    if ( slicedWorldGeometry->GetEvenlySpaced() )
    {
      mitk::Geometry2D *plane = slicedWorldGeometry->GetGeometry2D( 0 );

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
        slicedWorldGeometry->GetGeometry2D( s )->Project( point, projectedPoint );
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
  PlaneOperation op( OpORIENT, point, normal );

  m_CreatedWorldGeometry->ExecuteOperation( &op );

  this->SendCreatedWorldGeometryUpdate();
}


const mitk::TimeSlicedGeometry *
SliceNavigationController::GetCreatedWorldGeometry()
{
  return m_CreatedWorldGeometry;
}


const mitk::Geometry3D *
SliceNavigationController::GetCurrentGeometry3D()
{
  if ( m_CreatedWorldGeometry.IsNotNull() )
  {
    return m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos() );
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
      dynamic_cast< mitk::PlaneGeometry * >
        ( slicedGeometry->GetGeometry2D(this->GetSlice()->GetPos()) );
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
    if ( fabs( (float) direction[i] ) < 0.000000001 ) { ++c; }
    else { k = i; }
  }

  if ( c == 2 )
  {
    ScalarType min = m_InputWorldGeometry->GetOrigin()[k];
    ScalarType max = min + m_InputWorldGeometry->GetExtentInMM( k );

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
  if ( !operation )
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
    default:
    {
      // do nothing
      break;
    }
  }
}

// Relict from the old times, when automous descisions were accepted
// behaviour. Remains in here, because some RenderWindows do exist outide
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
            PointOperation* doOp = new mitk::PointOperation(OpMOVE, posEvent->GetWorldPosition());
            if (m_UndoEnabled)
            {
              m_OldPos = this->GetSlice()->GetPos();
              // m_OldPos holds the old slice position. For the undo controller this old position will be stored as index in mitk::PointOperation
              PointOperation* undoOp = new mitk::PointOperation(OpMOVE, posEvent->GetWorldPosition(), m_OldPos);
              OperationEvent *operationEvent = new mitk::OperationEvent(this, doOp, undoOp, "Move slices");
              m_UndoController->SetOperationEvent(operationEvent);
            }

            this->ExecuteOperation( doOp );

            // If click was performed in this render window than we have to update the status bar information about position and pixel value.
            if(baseRenderer == m_Renderer)
            {
              {
                std::string statusText;     
                TNodePredicateDataType<mitk::Image>::Pointer isImageData = TNodePredicateDataType<mitk::Image>::New();

                mitk::DataStorage::SetOfObjects::ConstPointer nodes = baseRenderer->GetDataStorage()->GetSubset(isImageData).GetPointer();
                mitk::Point3D worldposition = posEvent->GetWorldPosition();
                int  maxlayer = -32768;
                mitk::Image::Pointer image3D;
                // find image with largest layer, that is the image shown on top in the render window 
                for (unsigned int x = 0; x < nodes->size(); x++)
                {
                  if(nodes->at(x)->GetData()->GetGeometry()->IsInside(worldposition))
                  {
                    int layer = 0;
                    if(!(nodes->at(x)->GetIntProperty("layer", layer))) continue;
                    if(layer > maxlayer)
                    {
                      maxlayer = layer;
                      image3D = dynamic_cast<mitk::Image*>(nodes->at(x)->GetData());
                    }
                  }
                }
                
                std::stringstream stream;

                // get the position and gray value from the image and build up status bar text
                mitk::Point3D p;
                if(image3D.IsNotNull())
                {
                  image3D->GetGeometry()->WorldToIndex(worldposition, p);
                  stream<<"Position: <"<<p[0] << ", " << p[1] << ", " << p[2] << "> Pixel";
                  stream<<"; <"<<worldposition[0] << ", " << worldposition[1] << ", " << worldposition[2] << "> mm";
                  stream<<"; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: "<<image3D->GetPixelValue(p, baseRenderer->GetTimeStep())<<"  ";
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

