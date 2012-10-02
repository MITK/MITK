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


#include <mitkSlicesRotator.h>
#include <mitkSliceNavigationController.h>

#include <mitkStateEvent.h>
#include <mitkAction.h>
#include <mitkInteractionConst.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkRotationOperation.h>

#include <mitkBaseRenderer.h>
#include <mitkRenderingManager.h>

#include <mitkLine.h>
#include <mitkGeometry3D.h>
#include <mitkGeometry2D.h>
#include <mitkPlaneGeometry.h>
#include <mitkDisplayGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkTimeSlicedGeometry.h>

#include <vtkLinearTransform.h>

#include <math.h>

#include "rotate_cursor.xpm"

namespace mitk {

SlicesRotator::Pointer SlicesRotator::New()
{
  return SlicesRotator::New("slices-rotator");
}

SlicesRotator::SlicesRotator(const char* machine)
: SlicesCoordinator(machine)
{
  // make sure that AcSWITCHON and AcSWITCHOFF are defined int constants somewhere (e.g. mitkInteractionConst.h)
  CONNECT_ACTION( AcMOVE, DoSelectSlice );
  CONNECT_ACTION( AcCHECKPOINT, DoDecideBetweenRotationAndSliceSelection );
  CONNECT_ACTION( AcROTATESTART, DoStartRotation );
  CONNECT_ACTION( AcROTATE, DoRotationStep );
  CONNECT_ACTION( AcROTATEEND, DoEndRotation );
}

SlicesRotator::~SlicesRotator()
{

}


void SlicesRotator::OnSliceControllerAdded(SliceNavigationController* snc)
{
  if (!snc) return;

  snc->ConnectGeometrySendEvent(this); // connects creation of new world geometry to Self::SetGeometry
}

  
void SlicesRotator::OnSliceControllerRemoved(SliceNavigationController* snc)
{
  if (!snc) return;
  // nothing to do
}

/// Is called whenever a SliceNavigationController invokes an event. Will update the list
/// of SliceNavigationControllers that can handle rotation 
void SlicesRotator::SetGeometry(const itk::EventObject& /*EventObject*/)
{
  // there is no way to determine the sender?
  // ==> update whole list of SNCs
  UpdateRotatableSNCs();
}


void SlicesRotator::RotateToPoint( SliceNavigationController *rotationPlaneSNC, 
  SliceNavigationController *rotatedPlaneSNC,
  const Point3D &point, bool linked )
{
  SliceNavigationController *thirdSNC = NULL;

  SNCVector::iterator iter;
  for ( iter = m_RotatableSNCs.begin(); iter != m_RotatableSNCs.end(); ++iter )
  {
    if ( ((*iter) != rotationPlaneSNC)
      && ((*iter) != rotatedPlaneSNC) )
    {
      thirdSNC = *iter;
      break;
    }
  }

  if ( thirdSNC == NULL )
  {
    return;
  }

  const PlaneGeometry *rotationPlane = rotationPlaneSNC->GetCurrentPlaneGeometry();
  const PlaneGeometry *rotatedPlane = rotatedPlaneSNC->GetCurrentPlaneGeometry();
  const PlaneGeometry *thirdPlane = thirdSNC->GetCurrentPlaneGeometry();

  if ( (rotationPlane == NULL) || (rotatedPlane == NULL) 
    || (thirdPlane == NULL) )
  {
    return;
  }

  if ( rotatedPlane->DistanceFromPlane( point ) < 0.001 )
  {
    // Skip irrelevant rotations
    return;
  }

  Point3D projectedPoint;
  Line3D intersection;
  Point3D rotationCenter;

  if ( !rotationPlane->Project( point, projectedPoint )
    || !rotationPlane->IntersectionLine( rotatedPlane, intersection )
    || !thirdPlane->IntersectionPoint( intersection, rotationCenter ) )
  {
    return;
  }

  
  // All pre-requirements are met; execute the rotation

  Point3D referencePoint = intersection.Project( projectedPoint );


  Vector3D toProjected = referencePoint - rotationCenter;
  Vector3D toCursor    = projectedPoint - rotationCenter;
 
  // cross product: | A x B | = |A| * |B| * sin(angle)
  Vector3D axisOfRotation;
  vnl_vector_fixed< ScalarType, 3 > vnlDirection = 
    vnl_cross_3d( toCursor.GetVnlVector(), toProjected.GetVnlVector() );
  axisOfRotation.SetVnlVector( vnlDirection );
  
  // scalar product: A * B = |A| * |B| * cos(angle)
  // tan = sin / cos
  ScalarType angle = - atan2( 
    (double)(axisOfRotation.GetNorm()), 
    (double)(toCursor * toProjected) );
  angle *= 180.0 / vnl_math::pi;

  // create RotationOperation and apply to all SNCs that should be rotated
  RotationOperation op(OpROTATE, rotationCenter, axisOfRotation, angle);

  if ( !linked )
  {
    BaseRenderer *renderer = rotatedPlaneSNC->GetRenderer();
    if ( renderer == NULL )
    {
      return;
    }

    DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

    Point2D point2DWorld, point2DDisplayPre, point2DDisplayPost;
    displayGeometry->Map( rotationCenter, point2DWorld );
    displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPre );

    const Geometry3D *geometry3D = rotatedPlaneSNC->GetCreatedWorldGeometry();
    const TimeSlicedGeometry *timeSlicedGeometry = 
      dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
    if ( !timeSlicedGeometry )
    {
      return;
    }
    
    const_cast< TimeSlicedGeometry * >( timeSlicedGeometry )->ExecuteOperation( &op );

    displayGeometry->Map( rotationCenter, point2DWorld );
    displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPost );
    Vector2D vector2DDisplayDiff = point2DDisplayPost - point2DDisplayPre;

    //Vector2D origin = displayGeometry->GetOriginInMM();

    displayGeometry->MoveBy( vector2DDisplayDiff );

    rotatedPlaneSNC->SendCreatedWorldGeometryUpdate();
  }
  else
  {
    SNCVector::iterator iter;
    for ( iter = m_RotatableSNCs.begin(); iter != m_RotatableSNCs.end(); ++iter )
    {
      BaseRenderer *renderer = (*iter)->GetRenderer();
      if ( renderer == NULL )
      {
        continue;
      }

      DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

      Point2D point2DWorld, point2DDisplayPre, point2DDisplayPost;
      displayGeometry->Map( rotationCenter, point2DWorld );
      displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPre );

      const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
      const TimeSlicedGeometry *timeSlicedGeometry = 
        dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
      if ( !timeSlicedGeometry )
      {
        continue;
      }
      
      const_cast< TimeSlicedGeometry * >( timeSlicedGeometry )->ExecuteOperation( &op );

      displayGeometry->Map( rotationCenter, point2DWorld );
      displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPost );
      Vector2D vector2DDisplayDiff = point2DDisplayPost - point2DDisplayPre;

      //Vector2D origin = displayGeometry->GetOriginInMM();

      displayGeometry->MoveBy( vector2DDisplayDiff );

      (*iter)->SendCreatedWorldGeometryUpdate();
    } 
  }
} // end RotateToPoint


/// Updates the list of SliceNavigationControllers that can handle rotation 
void SlicesRotator::UpdateRotatableSNCs()
{
  m_RotatableSNCs.clear();

  for (SNCVector::iterator iter = m_SliceNavigationControllers.begin(); iter != m_SliceNavigationControllers.end(); ++iter)
  {
    const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
    const TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
    if (!timeSlicedGeometry) continue;

    const SlicedGeometry3D* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>( timeSlicedGeometry->GetGeometry3D(0) );
    if (!slicedGeometry) continue;

    if (slicedGeometry->IsValidSlice(0))  
    {
      // there were some lines of additional checks here in previous versions,
      // all of which would always evaluate to true, so the check was irrelevant.
      // Since the original intent was not documented, I removed all checks,
      // i.e. m_RotatableSNCs ends up being a list of all the registered
      // SliceNavigationControllers which have a SlicedGeometry3D with at least one slice,
      // which covers most standard cases.
      m_RotatableSNCs.push_back( *iter );
    }
  }
}

bool SlicesRotator::DoSelectSlice(Action* a, const StateEvent* e)
{
  // just reach through
  for (SNCVector::iterator iter = m_RotatableSNCs.begin(); iter != m_RotatableSNCs.end(); ++iter)
  {
    if ( !(*iter)->GetSliceLocked()  )
    {
      (*iter)->ExecuteAction(a,e);
    }
  }

  return true;
}

bool SlicesRotator::DoDecideBetweenRotationAndSliceSelection(Action*, const StateEvent* e)
{
  const ScalarType ThreshHoldDistancePixels = 12.0;

  // decide between moving and rotation 

  // for basic decision logic see class documentation.

  // Alle SNCs (Anzahl N) nach dem Abstand von posEvent->GetWorldPosition() zur aktuellen Ebene fragen.
  // Anzahl der Ebenen zaehlen, die naeher als ein gewisser Schwellwertwert sind -> nNah.
  // Wenn nNah == N
  //   Generiere ein PointEvent und schicke das an alle SNCs -> bewege den kreuz-mittelpunkt
  // Wenn nNah == 2
  //   Streiche stateEvent->Sender aus der Liste der nahen Ebenen
  //   fuer die uebrigen generiere eine RotationOperation und fuehre die aus
  // sonst
  //   
  const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(e->GetEvent());
  if (!posEvent) return false;

  Point3D cursor = posEvent->GetWorldPosition();
  //m_LastCursorPosition = cursor;

  unsigned int numNearPlanes = 0;
  m_SNCsToBeRotated.clear();
  Geometry2D* geometryToBeRotated = NULL;  // this one is grabbed
  Geometry2D* otherGeometry = NULL;        // this is also visible (for calculation of intersection)
  Geometry2D* clickedGeometry = NULL;      // the event originates from this one
  //SlicedGeometry3D* clickedSlicedGeometry;

  for (SNCVector::iterator iter = m_RotatableSNCs.begin(); iter != m_RotatableSNCs.end(); ++iter)
  {
    unsigned int slice = (*iter)->GetSlice()->GetPos();
    unsigned int time  = (*iter)->GetTime()->GetPos();

    const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
    const TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
    if (!timeSlicedGeometry) continue;

    const SlicedGeometry3D* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>( timeSlicedGeometry->GetGeometry3D(time) );
    if (!slicedGeometry) continue;

    Geometry2D* geometry2D = slicedGeometry->GetGeometry2D(slice);
    if (!geometry2D) continue; // this is not necessary?

    ScalarType distanceMM = geometry2D->Distance( cursor );

    BaseRenderer* renderer = e->GetEvent()->GetSender(); // TODO this is NOT SNC-specific! Should be!

    DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
    if (!displayGeometry) continue;

    ScalarType distancePixels = distanceMM / displayGeometry->GetScaleFactorMMPerDisplayUnit();
    if ( distancePixels <= ThreshHoldDistancePixels )
    {
      ++numNearPlanes; // count this one as a plane near to the cursor
    }

    if ( *iter == renderer->GetSliceNavigationController() ) // don't rotate the one where the user clicked
    {
      clickedGeometry = geometry2D;
      //clickedSlicedGeometry = const_cast<SlicedGeometry3D*>(slicedGeometry);
    }
    else
    {
      // @TODO here waits some bug to be found - maybe fixed by the || m_LinkPlanes in next line
      if ( (distancePixels <= ThreshHoldDistancePixels)
          && !(*iter)->GetSliceRotationLocked()
          && (m_SNCsToBeRotated.empty() || m_LinkPlanes) )
      {
        // this one is behind the clicked "line"
        m_SNCsToBeRotated.push_back(*iter);
        geometryToBeRotated = geometry2D;
      }
      else
      {
        otherGeometry = geometry2D;

        if ( m_LinkPlanes )
        {
          // All slices are rotated, i.e. the relative angles between
          // slices remain fixed
          m_SNCsToBeRotated.push_back(*iter);
        }
      }
    }
  }

  bool move (true);

  if ( geometryToBeRotated && otherGeometry && clickedGeometry
      && ( numNearPlanes == 2 ) )
  {
    // assure all three are valid, so calculation of center of rotation can be done
    move = false;
  }

  StateEvent *newStateEvent(NULL);

  // question in state machine is: "rotate?"
  if (move)
  {
    // move all planes to posEvent->GetWorldPosition()
    newStateEvent = new StateEvent(EIDNO, e->GetEvent());
  }
  else
  {
    // determine center of rotation TODO requires two plane geometries...
    PlaneGeometry* planeGeometry  = dynamic_cast<PlaneGeometry*>(clickedGeometry);
    PlaneGeometry* planeGeometry1 = dynamic_cast<PlaneGeometry*>(geometryToBeRotated);
    PlaneGeometry* planeGeometry2 = dynamic_cast<PlaneGeometry*>(otherGeometry);

    if (!planeGeometry || !planeGeometry1 || !planeGeometry2) return false; 

    Line3D intersection;
    if (!planeGeometry->IntersectionLine( planeGeometry1, intersection ))   return false;
    m_LastCursorPosition = intersection.Project(cursor);
    if (!planeGeometry2->IntersectionPoint(intersection, m_CenterOfRotation)) return false;
    // everything's fine
    newStateEvent = new StateEvent(EIDYES, e->GetEvent());

  }

  if (!newStateEvent) MITK_ERROR << "rotation would be nice but is impossible... " << std::endl;

  this->HandleEvent( newStateEvent );
  delete newStateEvent;

  return true;
}

bool SlicesRotator::DoStartRotation(Action*, const StateEvent*)
{
  this->SetMouseCursor( rotate_cursor_xpm, 0, 0 );
  this->InvokeEvent( SliceRotationEvent() ); // notify listeners
  return true;
}

bool SlicesRotator::DoEndRotation(Action*, const StateEvent*)
{
  this->ResetMouseCursor();
  this->InvokeEvent( SliceRotationEvent() ); // notify listeners
  return true;
}


bool SlicesRotator::DoRotationStep(Action*, const StateEvent* e)
{ // TODO check function
  const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(e->GetEvent());
  if (!posEvent) return false;

  Point3D cursor = posEvent->GetWorldPosition();

  Vector3D toProjected = m_LastCursorPosition - m_CenterOfRotation;
  Vector3D toCursor    = cursor - m_CenterOfRotation;

  // cross product: | A x B | = |A| * |B| * sin(angle)
// TODO use ITK CrossProduct
  Vector3D axisOfRotation;
  vnl_vector_fixed< ScalarType, 3 > vnlDirection = vnl_cross_3d( toCursor.GetVnlVector(), toProjected.GetVnlVector() );
  axisOfRotation.SetVnlVector(vnlDirection);

  // scalar product: A * B = |A| * |B| * cos(angle)
  // tan = sin / cos
  ScalarType angle = - atan2( (double)(axisOfRotation.GetNorm()), (double)(toCursor * toProjected) );
  angle *= 180.0 / vnl_math::pi;
  m_LastCursorPosition = cursor;

  // create RotationOperation and apply to all SNCs that should be rotated
  RotationOperation rotationOperation(OpROTATE, m_CenterOfRotation, axisOfRotation, angle);

  // iterate the OTHER slice navigation controllers: these are filled in DoDecideBetweenRotationAndSliceSelection
  for (SNCVector::iterator iter = m_SNCsToBeRotated.begin(); iter != m_SNCsToBeRotated.end(); ++iter)
  {
    //  - remember the center of rotation on the 2D display BEFORE rotation
    //  - execute rotation
    //  - calculate new center of rotation on 2D display
    //  - move display IF the center of rotation has moved slightly before and after rotation
    
    // DM 2012-10: this must probably be due to rounding errors only, right? 
    //             We don't have documentation on if/why this code is needed
    BaseRenderer *renderer = (*iter)->GetRenderer();
    if ( !renderer ) continue;

    DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

    Point2D rotationCenter2DWorld, point2DDisplayPreRotation, point2DDisplayPostRotation;
    displayGeometry->Map( m_CenterOfRotation, rotationCenter2DWorld );
    displayGeometry->WorldToDisplay( rotationCenter2DWorld, point2DDisplayPreRotation );

    const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
    const TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(geometry3D);
    if (!timeSlicedGeometry) continue;

    const_cast<TimeSlicedGeometry*>(timeSlicedGeometry)->ExecuteOperation(&rotationOperation);

    displayGeometry->Map( m_CenterOfRotation, rotationCenter2DWorld );
    displayGeometry->WorldToDisplay( rotationCenter2DWorld, point2DDisplayPostRotation );
    Vector2D vector2DDisplayDiff = point2DDisplayPostRotation - point2DDisplayPreRotation;

    displayGeometry->MoveBy( vector2DDisplayDiff );

    (*iter)->SendCreatedWorldGeometryUpdate();
  } 

  RenderingManager::GetInstance()->RequestUpdateAll();

  this->InvokeEvent( SliceRotationEvent() ); // notify listeners

  return true;
}

} // namespace 

