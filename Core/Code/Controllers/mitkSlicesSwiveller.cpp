/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 9252 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSlicesSwiveller.h"
#include "mitkSliceNavigationController.h"

#include "mitkStateEvent.h"
#include "mitkAction.h"
#include "mitkInteractionConst.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkRotationOperation.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkLine.h"
#include "mitkGeometry3D.h"
#include "mitkGeometry2D.h"
#include "mitkPlaneGeometry.h"
#include "mitkDisplayGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkTimeSlicedGeometry.h"

#include <math.h>


namespace mitk {

SlicesSwiveller::Pointer SlicesSwiveller::New()
{
  return SlicesSwiveller::New("slices-rotator");
}

SlicesSwiveller::SlicesSwiveller(const char* machine)
: SlicesCoordinator(machine),
  m_PreviousRotationAngle( 0.0 )
{
}

SlicesSwiveller::~SlicesSwiveller()
{

}


// check if the slices of this SliceNavigationController can be rotated (???) Possible
void SlicesSwiveller::OnSliceControllerAdded(SliceNavigationController* snc)
{
  if (!snc) return;

  // connects creation of new world geometry to Self::SetGeometry
  snc->ConnectGeometrySendEvent(this); 
}

  
void SlicesSwiveller::OnSliceControllerRemoved(SliceNavigationController* snc)
{
  if (!snc) return;
  // nothing to do
}

/// Is called whenever a SliceNavigationController invokes an event. Will 
// update the list of SliceNavigationControllers that can handle rotation 
void SlicesSwiveller::SetGeometry(const itk::EventObject& /*EventObject*/)
{
  // there is no way to determine the sender?
  // ==> update whole list of SNCs
  UpdateRelevantSNCs();
}

/// Updates the list of SliceNavigationControllers that can handle rotation 
void SlicesSwiveller::UpdateRelevantSNCs()
{
  m_RelevantSNCs.clear();

  SNCVector::iterator iter;
  for ( iter = m_SliceNavigationControllers.begin(); 
        iter != m_SliceNavigationControllers.end(); 
        ++iter)
  {
    const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
    const TimeSlicedGeometry* timeSlicedGeometry = 
      dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
    
    if (!timeSlicedGeometry) continue;

    const SlicedGeometry3D* slicedGeometry = 
      dynamic_cast<const SlicedGeometry3D*>( 
        timeSlicedGeometry->GetGeometry3D(0) );
    if (!slicedGeometry) continue;

    Geometry2D *firstSlice( NULL );
    Geometry2D *secondSlice( NULL );
    
    if (slicedGeometry->IsValidSlice(0))
    {
      firstSlice = slicedGeometry->GetGeometry2D(0);
    }
    if (slicedGeometry->IsValidSlice(1)) 
    {
      secondSlice = slicedGeometry->GetGeometry2D(1);
    }
    
    // If the direction vector of these two slices is the same, then accept 
    // this slice stack as rotatable
    Vector3D right1 = firstSlice->GetAxisVector(0);
    Vector3D up1 = firstSlice->GetAxisVector(1);
    vnl_vector_fixed< ScalarType, 3 > vnlDirection1 = 
      vnl_cross_3d(right1.GetVnlVector(), up1.GetVnlVector());
    Vector3D direction1;
    direction1.SetVnlVector(vnlDirection1);
    
    Vector3D right2 = firstSlice->GetAxisVector(0);
    Vector3D up2 = firstSlice->GetAxisVector(1);
    vnl_vector_fixed< ScalarType, 3 > vnlDirection2 = 
      vnl_cross_3d(right2.GetVnlVector(), up2.GetVnlVector());
    Vector3D direction2;
    direction2.SetVnlVector(vnlDirection2);

    bool equal = true;
    const ScalarType eps = 0.0001;
    for (int i = 0; i < 3; ++i)
    {
      if ( fabs(direction1[i] - direction2[i]) > eps ) 
      {
        equal = false;
      }
    }

    if (equal) // equal direction vectors
    {
      m_RelevantSNCs.push_back( *iter );
    }
  }
}

bool SlicesSwiveller
::ExecuteAction(Action* action, StateEvent const* stateEvent)
{
  const ScalarType ThresholdDistancePixels = 6.0;
  
  bool ok = false;
  
  switch ( action->GetActionId() )
  {
    case AcMOVE:
    {
      // just reach through
      SNCVector::iterator iter;
      for ( iter = m_RelevantSNCs.begin(); 
            iter != m_RelevantSNCs.end(); 
            ++iter )
      {
        if ( !(*iter)->GetSliceRotationLocked()  )
        {
          (*iter)->ExecuteAction(action, stateEvent);
        }
      }
      
      ok = true;
      break;
    }
    case AcROTATE:
    {
      const DisplayPositionEvent *posEvent = 
        dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

      if (!posEvent) break;
      
      // Determine relative mouse movement projected onto world space
      Point2D cursor = posEvent->GetDisplayPosition();
      Vector2D relativeCursor = cursor - m_ReferenceCursor;
      Vector3D relativeCursorAxis = 
        m_RotationPlaneXVector * relativeCursor[0]
        + m_RotationPlaneYVector * relativeCursor[1];

      // Determine rotation axis (perpendicular to rotation plane and cursor 
      // movement)
      Vector3D rotationAxis = itk::CrossProduct( 
        m_RotationPlaneNormal, relativeCursorAxis );

      ScalarType rotationAngle = relativeCursor.GetNorm() / 2.0;

      // Restore the initial plane pose by undoing the previous rotation
      // operation
      RotationOperation op( OpROTATE, m_CenterOfRotation, 
        m_PreviousRotationAxis, -m_PreviousRotationAngle );

      SNCVector::iterator iter;
      for ( iter = m_SNCsToBeRotated.begin(); 
            iter != m_SNCsToBeRotated.end(); 
            ++iter )
      {
        if ( !(*iter)->GetSliceRotationLocked() )
        {
          const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
          const TimeSlicedGeometry* timeSlicedGeometry = 
            dynamic_cast<const TimeSlicedGeometry*>(geometry3D);
          if (!timeSlicedGeometry) continue;
          
          const_cast<TimeSlicedGeometry*>(timeSlicedGeometry)
            ->ExecuteOperation(&op);
          
          (*iter)->SendCreatedWorldGeometryUpdate();
        }
      } 

      // Apply new rotation operation to all relevant SNCs
      RotationOperation op2( OpROTATE, m_CenterOfRotation, 
        rotationAxis, rotationAngle );

      for ( iter = m_SNCsToBeRotated.begin(); 
            iter != m_SNCsToBeRotated.end(); 
            ++iter)
      {
        if ( !(*iter)->GetSliceRotationLocked() )
        {
          //// Map rotation center onto display geometry (will be used as
          //// pre-rotation vector for compensating a visual shift of the
          //// rotation center)
          //BaseRenderer *renderer = (*iter)->GetRenderer();
          //DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

          //Point2D point2DWorld, point2DDisplayPre, point2DDisplayPost;
          //displayGeometry->Map( m_CenterOfRotation, point2DWorld );
          //displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPre );

          // Retrieve the TimeSlicedGeometry of this SliceNavigationController
          const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
          const TimeSlicedGeometry* timeSlicedGeometry = 
            dynamic_cast<const TimeSlicedGeometry*>(geometry3D);
          if (!timeSlicedGeometry) continue;

          // Execute the new rotation
          const_cast<TimeSlicedGeometry*>(timeSlicedGeometry)
            ->ExecuteOperation(&op2);

          //// After rotation: map rotation center onto new display geometry...
          //displayGeometry->Map( m_CenterOfRotation, point2DWorld );
          //displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPost );
          //Vector2D vector2DDisplayDiff = point2DDisplayPost - point2DDisplayPre;

          //// And use the difference between pre- and post-rotation vectors to
          //// compensate for display geometry shift:
          //Vector2D origin = displayGeometry->GetOriginInMM();
          //displayGeometry->MoveBy( vector2DDisplayDiff );


          // Notify listeners
          (*iter)->SendCreatedWorldGeometryUpdate();
        }
      } 

      m_PreviousRotationAxis = rotationAxis;
      m_PreviousRotationAngle = rotationAngle;


      RenderingManager::GetInstance()->RequestUpdateAll();
      
      this->InvokeEvent( SliceRotationEvent() ); // notify listeners
      
      ok = true;
      break;
    }
    case AcCHECKPOINT:
    {
      // Decide between moving and rotation: if we're close to the crossing
      // point of the planes, moving mode is entered, otherwise
      // rotation/swivel mode
      const DisplayPositionEvent *posEvent = 
        dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

      BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();

      if ( !posEvent || !renderer ) 
      {
        break;
      }


      const Point3D &cursor = posEvent->GetWorldPosition();
      
      m_SNCsToBeRotated.clear();
      
      const PlaneGeometry *clickedGeometry( NULL );
      const PlaneGeometry *otherGeometry1( NULL );
      const PlaneGeometry *otherGeometry2( NULL );
      
      SNCVector::iterator iter;
      for ( iter = m_RelevantSNCs.begin(); iter != m_RelevantSNCs.end(); ++iter )
      {
        //unsigned int slice = (*iter)->GetSlice()->GetPos();
        //unsigned int time  = (*iter)->GetTime()->GetPos();
        
        const PlaneGeometry *planeGeometry = (*iter)->GetCurrentPlaneGeometry();
        if ( !planeGeometry ) continue;

        if ( *iter == renderer->GetSliceNavigationController() )
        {
          clickedGeometry = planeGeometry;
          m_SNCsToBeRotated.push_back(*iter);
        }
        else
        {
          if ( otherGeometry1 == NULL )
          {
            otherGeometry1 = planeGeometry;
          }
          else
          {
            otherGeometry2 = planeGeometry;
          }
          if ( m_LinkPlanes )
          {
            // If planes are linked, apply rotation to all planes
            m_SNCsToBeRotated.push_back(*iter);
          }
        }
      }

      StateEvent *newStateEvent( NULL );

      mitk::Line3D line;
      mitk::Point3D point;
      if ( (clickedGeometry != NULL) && (otherGeometry1 != NULL) 
        && (otherGeometry2 != NULL)
        && clickedGeometry->IntersectionLine( otherGeometry1, line )
        && otherGeometry2->IntersectionPoint( line, point ))
      {
        m_CenterOfRotation = point;
        if ( m_CenterOfRotation.EuclideanDistanceTo( cursor ) 
           < ThresholdDistancePixels )
        {
          newStateEvent = new StateEvent(EIDNO, stateEvent->GetEvent());
        }
        else
        {
          m_ReferenceCursor = posEvent->GetDisplayPosition();

          // Get main axes of rotation plane and store it for rotation step
          m_RotationPlaneNormal = clickedGeometry->GetNormal();

          ScalarType xVector[] = { 1.0, 0.0, 0.0 };
          ScalarType yVector[] = { 0.0, 1.0, 0.0 };
          clickedGeometry->Geometry3D::IndexToWorld( 
            Point3D(), Vector3D( xVector), m_RotationPlaneXVector );
          clickedGeometry->Geometry3D::IndexToWorld( 
            Point3D(), Vector3D( yVector), m_RotationPlaneYVector );

          m_RotationPlaneNormal.Normalize();
          m_RotationPlaneXVector.Normalize();
          m_RotationPlaneYVector.Normalize();

          m_PreviousRotationAxis.Fill( 0.0 );
          m_PreviousRotationAxis[2] = 1.0;
          m_PreviousRotationAngle = 0.0;

          newStateEvent = new StateEvent(EIDYES, stateEvent->GetEvent());
        }
      }
      else
      {
        newStateEvent = new StateEvent(EIDNO, stateEvent->GetEvent());
      }
    
      this->HandleEvent( newStateEvent );
      delete newStateEvent;

      ok = true;
      break;
    }
    case AcROTATESTART:
    {
      this->InvokeEvent( SliceRotationEvent() ); // notify listeners
      break;
    }
    case AcROTATEEND:
    {
      this->InvokeEvent( SliceRotationEvent() ); // notify listeners
      break;
    }
    default:
    {
      break;
    }
  }

  return ok;
}

} // namespace 

