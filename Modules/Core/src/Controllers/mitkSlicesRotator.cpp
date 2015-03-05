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
#include <mitkPlaneGeometry.h>
#include <mitkPlaneGeometry.h>
#include <mitkDisplayGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkAbstractTransformGeometry.h>
#include <mitkDisplayGeometry.h>

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
    // nothing to do, base class does the bookkeeping
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
    MITK_WARN << "Deprecated function! Use SliceNavigationController::ReorientSlices() instead";

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

      TimeGeometry *timeGeometry= rotatedPlaneSNC->GetCreatedWorldGeometry();
      if ( !timeGeometry )
      {
        return;
      }

      timeGeometry->ExecuteOperation( &op );

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

        TimeGeometry* timeGeometry = (*iter)->GetCreatedWorldGeometry();
        if ( !timeGeometry )
        {
          continue;
        }

        timeGeometry->ExecuteOperation( &op );

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
      const TimeGeometry* timeGeometry = (*iter)->GetCreatedWorldGeometry();
      if (!timeGeometry) continue;

      const SlicedGeometry3D* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>( timeGeometry->GetGeometryForTimeStep(0).GetPointer() );
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
    // Decide between moving and rotation slices.
    // For basic decision logic see class documentation.

    /*
    Detail logic:

    1. Find the SliceNavigationController that has sent the event: this one defines our rendering plane and will NOT be rotated. Needs not even be counted or checked.
    2. Inspect every other SliceNavigationController
    - calculate the line intersection of this SliceNavigationController's plane with our rendering plane
    - if there is NO interesection, ignore and continue
    - IF there is an intersection
    - check the mouse cursor's distance from that line.
    0. if the line is NOT near the cursor, remember the plane as "one of the other planes" (which can be rotated in "locked" mode)
    1. on first line near the cursor,  just remember this intersection line as THE other plane that we want to rotate
    2. on every consecutive line near the cursor, check if the line is geometrically identical to the line that we want to rotate
    - if yes, we just push this line to the "other" lines and rotate it along
    - if no, then we have a situation where the mouse is near two other lines (e.g. crossing point) and don't want to rotate
    */
    const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(e->GetEvent());
    if (!posEvent) return false;

    BaseRenderer* clickedRenderer = e->GetEvent()->GetSender();
    const PlaneGeometry* ourViewportGeometry = dynamic_cast<const PlaneGeometry*>( clickedRenderer->GetCurrentWorldPlaneGeometry() );
    // These sanity checks were introduced with bug 17877, since plane geometries are now a shared base class of several geometries
    // They may ultimately be unecessary

    const mitk::AbstractTransformGeometry* abstractGeometry = dynamic_cast< const AbstractTransformGeometry * > (ourViewportGeometry);
    if (abstractGeometry != NULL) MITK_WARN << "SliceRotator recieved an AbstractTransformGeometry, expecting a simple PlainGeometry, behaviour should be verified.";
    const mitk::DisplayGeometry* displayGeometry = dynamic_cast< const DisplayGeometry * > (ourViewportGeometry);
    if (displayGeometry != NULL) MITK_WARN << "SliceRotator recieved a DisplayGeometry, expecting a simple PlainGeometry, behaviour should be verified.";
    // End sanity checks

    if (!ourViewportGeometry) return false;

    DisplayGeometry* clickedDisplayGeometry = clickedRenderer->GetDisplayGeometry();
    if (!clickedDisplayGeometry) return false;

    MITK_DEBUG << "=============================================";
    MITK_DEBUG << "Renderer under cursor is " << clickedRenderer->GetName();

    Point3D cursorPosition = posEvent->GetWorldPosition();
    const PlaneGeometry* geometryToBeRotated = NULL;  // this one is under the mouse cursor
    const PlaneGeometry* anyOtherGeometry = NULL;    // this is also visible (for calculation of intersection ONLY)
    Line3D intersectionLineWithGeometryToBeRotated;

    bool hitMultipleLines(false);
    m_SNCsToBeRotated.clear();

    const double threshholdDistancePixels = 12.0;

    for (SNCVector::iterator iter = m_RotatableSNCs.begin(); iter != m_RotatableSNCs.end(); ++iter)
    {
      // If the mouse cursor is in 3D Renderwindow, do not check for intersecting planes.
      if (clickedRenderer->GetMapperID() == BaseRenderer::Standard3D)
        break;

      const PlaneGeometry* otherRenderersRenderPlane = (*iter)->GetCurrentPlaneGeometry();
      if (otherRenderersRenderPlane == NULL) continue; // ignore, we don't see a plane
      MITK_DEBUG << "  Checking plane of renderer " << (*iter)->GetRenderer()->GetName();

      // check if there is an intersection
      Line3D intersectionLine; // between rendered/clicked geometry and the one being analyzed
      if (!ourViewportGeometry->IntersectionLine( otherRenderersRenderPlane, intersectionLine ))
      {
        continue; // we ignore this plane, it's parallel to our plane
      }

      // check distance from intersection line
      double distanceFromIntersectionLine = intersectionLine.Distance( cursorPosition );
      ScalarType distancePixels = distanceFromIntersectionLine / clickedDisplayGeometry->GetScaleFactorMMPerDisplayUnit();
      MITK_DEBUG << "    Distance of plane from cursor " << distanceFromIntersectionLine << " mm, which is around " << distancePixels << " px" ;

      // far away line, only remember for linked rotation if necessary
      if (distanceFromIntersectionLine > threshholdDistancePixels)
      {
        MITK_DEBUG << "    Plane is too far away --> remember as otherRenderersRenderPlane";
        anyOtherGeometry = otherRenderersRenderPlane; // we just take the last one, so overwrite each iteration (we just need some crossing point)
        // TODO what about multiple crossings? NOW we have undefined behavior / random crossing point is used

        if (m_LinkPlanes)
        {
          m_SNCsToBeRotated.push_back(*iter);
        }
      }
      else // close to cursor
      {
        MITK_DEBUG << "    Plane is close enough to cursor...";
        if ( geometryToBeRotated == NULL ) // first one close to the cursor
        {
          MITK_DEBUG << "    It is the first close enough geometry, remember as geometryToBeRotated";
          geometryToBeRotated = otherRenderersRenderPlane;
          intersectionLineWithGeometryToBeRotated = intersectionLine;
          m_SNCsToBeRotated.push_back(*iter);
        }
        else
        {
          MITK_DEBUG << "    Second or later close enough geometry";
          // compare to the line defined by geometryToBeRotated: if identical, just rotate this otherRenderersRenderPlane together with the primary one
          //                                                     if different, DON'T rotate

          if ( intersectionLine.IsParallel( intersectionLineWithGeometryToBeRotated )
            && intersectionLine.Distance( intersectionLineWithGeometryToBeRotated.GetPoint1() ) < mitk::eps )
          {
            MITK_DEBUG << "    This line is the same as intersectionLineWithGeometryToBeRotated which we already know";
            m_SNCsToBeRotated.push_back(*iter);
          }
          else
          {
            MITK_DEBUG << "    This line is NOT the same as intersectionLineWithGeometryToBeRotated which we already know";
            hitMultipleLines = true;
          }
        }
      }
    }

    bool moveSlices(true);

    if ( geometryToBeRotated && anyOtherGeometry && ourViewportGeometry && !hitMultipleLines )
    {
      // assure all three are valid, so calculation of center of rotation can be done
      moveSlices = false;
    }

    MITK_DEBUG << "geometryToBeRotated:   " << (void*)geometryToBeRotated;
    MITK_DEBUG << "anyOtherGeometry:    " << (void*)anyOtherGeometry;
    MITK_DEBUG << "ourViewportGeometry: " << (void*)ourViewportGeometry;
    MITK_DEBUG << "hitMultipleLines?    " << hitMultipleLines;
    MITK_DEBUG << "moveSlices?          " << moveSlices;

    std::auto_ptr<StateEvent> decidedEvent;

    // question in state machine is: "rotate?"
    if (moveSlices) // i.e. NOT rotate
    {
      // move all planes to posEvent->GetWorldPosition()
      decidedEvent.reset( new StateEvent(EIDNO, e->GetEvent()) );
      MITK_DEBUG << "Rotation not possible, not enough information (other planes crossing rendering plane) ";
    }
    else
    { // we DO have enough information for rotation
      m_LastCursorPosition = intersectionLineWithGeometryToBeRotated.Project(cursorPosition); // remember where the last cursor position ON THE LINE has been observed

      if (anyOtherGeometry->IntersectionPoint(intersectionLineWithGeometryToBeRotated, m_CenterOfRotation)) // find center of rotation by intersection with any of the OTHER lines
      {
        decidedEvent.reset( new StateEvent(EIDYES, e->GetEvent()) );
        MITK_DEBUG << "Rotation possible";
      }
      else
      {
        MITK_DEBUG << "Rotation not possible, cannot determine the center of rotation!?";
        decidedEvent.reset( new StateEvent(EIDNO, e->GetEvent()) );
      }
    }

    this->HandleEvent( decidedEvent.get() );

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
  {
    const DisplayPositionEvent* posEvent = dynamic_cast<const DisplayPositionEvent*>(e->GetEvent());
    if (!posEvent) return false;

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

      TimeGeometry* timeGeometry = (*iter)->GetCreatedWorldGeometry();
      if (!timeGeometry) continue;

      timeGeometry->ExecuteOperation(&rotationOperation);

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