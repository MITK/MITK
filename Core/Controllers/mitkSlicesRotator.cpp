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


#include <mitkSlicesRotator.h>
#include <mitkSliceNavigationController.h>

#include <mitkStateEvent.h>
#include <mitkAction.h>
#include <mitkInteractionConst.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkRotationOperation.h>

#include <mitkBaseRenderer.h>
#include <mitkRenderWindow.h>
#include <mitkRenderingManager.h>

#include <mitkLine.h>
#include <mitkGeometry3D.h>
#include <mitkGeometry2D.h>
#include <mitkPlaneGeometry.h>
#include <mitkDisplayGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkTimeSlicedGeometry.h>

#include <mitkApplicationCursor.h>

#include <vtkLinearTransform.h>

#include <math.h>

#include "rotate_cursor.xpm"

namespace mitk {

SlicesRotator::Pointer SlicesRotator::New()
{
  // from itkNewMacro()
  Pointer smartPtr;
  SlicesRotator* rawPtr = new SlicesRotator("slices-rotator");
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  return smartPtr;
}

SlicesRotator::SlicesRotator(const char* machine)
: SlicesCoordinator(machine)
{

}

SlicesRotator::~SlicesRotator()
{

}


// check if the slices of this SliceNavigationController can be rotated (???) Possible
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
  UpdateRelevantSNCs();
}

/// Updates the list of SliceNavigationControllers that can handle rotation 
void SlicesRotator::UpdateRelevantSNCs()
{
  m_RelevantSNCs.clear();

  for (SNCVector::iterator iter = m_SliceNavigationControllers.begin(); iter != m_SliceNavigationControllers.end(); ++iter)
  {
    const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
    const TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>( geometry3D );
    if (!timeSlicedGeometry) continue;

    const SlicedGeometry3D* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>( timeSlicedGeometry->GetGeometry3D(0) );
    if (!slicedGeometry) continue;

    Geometry2D* firstSlice(NULL);
    Geometry2D* secondSlice(NULL);
    if (slicedGeometry->IsValidSlice(0))  firstSlice = slicedGeometry->GetGeometry2D(0);
    if (slicedGeometry->IsValidSlice(1)) secondSlice = slicedGeometry->GetGeometry2D(1);
    
    // if the direction vector of these two slices is the same, then accept this slice stack as rotatable
    Vector3D right1 = firstSlice->GetAxisVector(0);
    Vector3D up1 = firstSlice->GetAxisVector(1);
    vnl_vector_fixed< ScalarType, 3 > vnlDirection1 = vnl_cross_3d(right1.GetVnlVector(), up1.GetVnlVector());
    Vector3D direction1;
    direction1.SetVnlVector(vnlDirection1);
    
    Vector3D right2 = firstSlice->GetAxisVector(0);
    Vector3D up2 = firstSlice->GetAxisVector(1);
    vnl_vector_fixed< ScalarType, 3 > vnlDirection2 = vnl_cross_3d(right2.GetVnlVector(), up2.GetVnlVector());
    Vector3D direction2;
    direction2.SetVnlVector(vnlDirection2);

    bool equal = true;
    const ScalarType eps = 0.0001;
    for (int i = 0; i < 3; ++i)
      if ( fabs(direction1[i] - direction2[i]) > eps ) 
        equal = false;

    if (equal) // equal direction vectors
    {
      m_RelevantSNCs.push_back( *iter );
    }
  }
}

bool SlicesRotator::ExecuteAction(Action* action, StateEvent const* stateEvent)
{
  const ScalarType ThreshHoldDistancePixels = 6;
  
  bool ok = false;
  
  switch ( action->GetActionId() )
  {
    case AcMOVE:
    {
      // just reach through
      for (SNCVector::iterator iter = m_RelevantSNCs.begin(); iter != m_RelevantSNCs.end(); ++iter)
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
      const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
      if (!posEvent) break;
      
      Point3D cursor = posEvent->GetWorldPosition();
  
      Vector3D toProjected = m_LastCursorPosition - m_CenterOfRotation;
      Vector3D toCursor    = cursor - m_CenterOfRotation;
     
      // cross product: | A x B | = |A| * |B| * sin(angle)
      Vector3D axisOfRotation;
      vnl_vector_fixed< ScalarType, 3 > vnlDirection = vnl_cross_3d( toCursor.GetVnlVector(), toProjected.GetVnlVector() );
      axisOfRotation.SetVnlVector(vnlDirection);
      
      // scalar product: A * B = |A| * |B| * cos(angle)
      // tan = sin / cos
      ScalarType angle = - atan2( (double)(axisOfRotation.GetNorm()), (double)(toCursor * toProjected) );
      angle *= 180.0 / vnl_math::pi;
      m_LastCursorPosition = cursor;

      // create RotationOperation and apply to all SNCs that should be rotated
      RotationOperation op(OpROTATE, m_CenterOfRotation, axisOfRotation, angle);

      // TEST
      int i = 0;

      for (SNCVector::iterator iter = m_SNCsToBeRotated.begin(); iter != m_SNCsToBeRotated.end(); ++iter)
      {
        if ( !(*iter)->GetSliceRotationLocked() )
        {
          BaseRenderer *renderer = (*iter)->GetRenderer();
          DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();

          std::cout << i << ":" << std::endl;

          Point2D point2DWorld, point2DDisplayPre, point2DDisplayPost;
          displayGeometry->Map( m_CenterOfRotation, point2DWorld );
          displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPre );

          std::cout << "  WorldPre: " << point2DWorld << " / DisplayPre: " << point2DDisplayPre << std::endl;

          const Geometry3D* geometry3D = (*iter)->GetCreatedWorldGeometry();
          const TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(geometry3D);
          if (!timeSlicedGeometry) continue;
          
          const_cast<TimeSlicedGeometry*>(timeSlicedGeometry)->ExecuteOperation(&op);

          //vtkLinearTransform *inverseTransformVtk = 
          //  displayGeometry->GetVtkTransform()->GetLinearInverse();

          //ScalarType pvtkCenterOfRotation[3];
          //pvtkCenterOfRotation[0] = m_CenterOfRotation[0];
          //pvtkCenterOfRotation[1] = m_CenterOfRotation[1];
          //pvtkCenterOfRotation[2] = m_CenterOfRotation[2];

          //ScalarType scaleFactorMMPerUnitX = 
          //  displayGeometry->GetExtentInMM(0) / displayGeometry->GetExtent(0);
          //ScalarType scaleFactorMMPerUnitY = 
          //  displayGeometry->GetExtentInMM(1) / displayGeometry->GetExtent(1);  

          //ScalarType scaleFactorMMPerDisplayUnit = displayGeometry->GetScaleFactorMMPerDisplayUnit();
          //Vector2D &originInMM = displayGeometry->GetOriginInMM();

          ////displayGeometry->Map( m_CenterOfRotation, point2DWorld );

          //ScalarType pvtkDisplayPost[3];
          //inverseTransformVtk->TransformPoint( pvtkCenterOfRotation, pvtkDisplayPost );

          //pvtkDisplayPost[0] *= scaleFactorMMPerUnitX;
          //pvtkDisplayPost[1] *= scaleFactorMMPerUnitY;

          ////displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPost );
          //pvtkDisplayPost[0] -= originInMM[0];
          //pvtkDisplayPost[1] -= originInMM[1];

          //pvtkDisplayPost[0] /= scaleFactorMMPerDisplayUnit;
          //pvtkDisplayPost[1] /= scaleFactorMMPerDisplayUnit;

          //point2DDisplayPost[0] = pvtkDisplayPost[0];
          //point2DDisplayPost[1] = pvtkDisplayPost[1];


          displayGeometry->Map( m_CenterOfRotation, point2DWorld );
          displayGeometry->WorldToDisplay( point2DWorld, point2DDisplayPost );
          Vector2D vector2DDisplayDiff = point2DDisplayPost - point2DDisplayPre;

          Vector2D origin = displayGeometry->GetOriginInMM();
          std::cout << "  WorldPost: " << point2DWorld << " / DisplayPost: " << point2DDisplayPost << std::endl;
          std::cout << "  Diff   - " << vector2DDisplayDiff << std::endl;
          std::cout << "  Origin - " << origin << std::endl;
          ++i;

          displayGeometry->MoveBy( vector2DDisplayDiff );

          (*iter)->SendCreatedWorldGeometryUpdate();
        }
      } 
      std::cout << "--------------------------------" << std::endl;

      
      
      // TEST
      //BaseRenderer* renderer = stateEvent->GetEvent()->GetSender(); // TODO this is NOT SNC-specific! Should be!
      //
      //DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
      //if (!displayGeometry) break;

      //Point2D point2DWorld, point2DDisplay;
      //displayGeometry->Map( m_CenterOfRotation, point2DWorld );
      //displayGeometry->WorldToDisplay( point2DWorld, point2DDisplay );

      //std::cout << "RotationCenter: " << m_CenterOfRotation << std::endl;
      //std::cout << "PointWorld:     " << point2DWorld << std::endl;
      //std::cout << "PointDisplay:   " << point2DDisplay << std::endl;
      //std::cout << "--------------------------------------------" << std::endl;



      RenderingManager::GetInstance()->RequestUpdateAll();
      
      ok = true;
      break;
    }
    case AcCHECKPOINT:
    {
      // decide between moving and rotation 

      // Alle SNCs (Anzahl N) nach dem Abstand von posEvent->GetWorldPosition() zur aktuellen Ebene fragen.
      // Anzahl der Ebenen zaehlen, die naeher als ein gewisser Schwellwertwert sind -> nNah.
      // Wenn nNah == N
      //   Generiere ein PointEvent und schicke das an alle SNCs -> bewege den kreuz-mittelpunkt
      // Wenn nNah == 2
      //   Streiche stateEvent->Sender aus der Liste der nahen Ebenen
      //   fuer die uebrigen generiere eine RotationOperation und fuehre die aus
      // sonst
      //   
      const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
      if (!posEvent) break;

      Point3D cursor = posEvent->GetWorldPosition();
      //m_LastCursorPosition = cursor;
      
      unsigned int numNearPlanes = 0;
      m_SNCsToBeRotated.clear();
      Geometry2D* geometryToBeRotated = NULL;  // this one is grabbed
      Geometry2D* otherGeometry = NULL;        // this is also visible (for calculation of intersection)
      Geometry2D* clickedGeometry = NULL;      // the event originates from this one
      SlicedGeometry3D* clickedSlicedGeometry;

      for (SNCVector::iterator iter = m_RelevantSNCs.begin(); iter != m_RelevantSNCs.end(); ++iter)
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

        BaseRenderer* renderer = stateEvent->GetEvent()->GetSender(); // TODO this is NOT SNC-specific! Should be!
        
        DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
        if (!displayGeometry) continue;
        
        ScalarType distancePixels = distanceMM / displayGeometry->GetScaleFactorMMPerDisplayUnit();
        if ( distancePixels <= ThreshHoldDistancePixels )
        {
          ++numNearPlanes; // count this one as a plane near to the cursor
        }
       
        if ( *iter == renderer->GetRenderWindow()->GetSliceNavigationController() ) // don't rotate the one where the user clicked
        {
          clickedGeometry = geometry2D;
          clickedSlicedGeometry = const_cast<SlicedGeometry3D*>(slicedGeometry);
        }
        else
        {
          // @TODO here waits some bug to be found - maybe fixed by the || m_LinkPlanes in next line
          if ( distancePixels <= ThreshHoldDistancePixels && (m_SNCsToBeRotated.empty() || m_LinkPlanes) )
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

      if ( geometryToBeRotated && otherGeometry && clickedGeometry ) // assure all three are valid, so calculation of center of rotation can be done
      if ( numNearPlanes == 2 ) 
        move = false;
      
      StateEvent* newStateEvent(NULL);

      // question in state machine is: "rotate?"
      if (move)
      {
        // move all planes to posEvent->GetWorldPosition()
        newStateEvent = new StateEvent(EIDNO, stateEvent->GetEvent());
      }
      else
      {
        // determine center of rotation TODO requires two plane geometries...
        PlaneGeometry* planeGeometry  = dynamic_cast<PlaneGeometry*>(clickedGeometry);
        PlaneGeometry* planeGeometry1 = dynamic_cast<PlaneGeometry*>(geometryToBeRotated);
        PlaneGeometry* planeGeometry2 = dynamic_cast<PlaneGeometry*>(otherGeometry);

        if (!planeGeometry || !planeGeometry1 || !planeGeometry2) break; 
        
        Line3D intersection;
        if (!planeGeometry->IntersectionLine( planeGeometry1, intersection ))   break;
        m_LastCursorPosition = intersection.Project(cursor);
        if (!planeGeometry2->IntersectionPoint(intersection, m_CenterOfRotation)) break;
        // everything's fine
        newStateEvent = new StateEvent(EIDYES, stateEvent->GetEvent());

      }

      if (!newStateEvent) std::cerr << "rotation would be nice but is impossible... (in l. " << __LINE__ << " of " << __FILE__ << std::endl;
      
      this->HandleEvent( newStateEvent );
      delete newStateEvent;

      ok = true;
      break;
    }
    case AcROTATESTART:
    {
      ApplicationCursor::GetInstance()->PushCursor(rotate_cursor_xpm, 0, 0);
      break;
    }
    case AcROTATEEND:
    {
      ApplicationCursor::GetInstance()->PopCursor();
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

