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


#ifndef SLICESROTATOR_H_HEADER_INCLUDED_C1C55A2F
#define SLICESROTATOR_H_HEADER_INCLUDED_C1C55A2F

#include <mitkSlicesCoordinator.h>
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop
#include <mitkNumericTypes.h>

namespace mitk {

/**
  \brief Coordinates rotation of multiple visible rendering planes (represented as lines in other render windows).
  \ingroup NavigationControl

  This class takes care of several SliceNavigationControllers and handles slice selection
  / slice rotation. It is added as listener to GlobalInteraction by QmitkStdMultiWidget.

  The SlicesRotator class adds the possibility of slice rotation to the "normal" behaviour
  of SliceNavigationControllers (which is picking one plane from a stack of planes).

  This additional class SlicesRotator is needed, because one has to be aware of multiple
  "visible slices" (selected PlaneGeometries of some SliceNavigationControllers) in order to
  choose between rotation and slice selection. Such functionality could not be implemented
  by a single SliceNavigationController.

  Rotation is achieved by modifying (rotating) the generated TimeGeometry of the
  corresponding SliceNavigationControllers.

  \section mitkSlicesRotator_StandardCase The standard case: three orthogonal views (MPR)

  With SlicesRotator, the rule to choose between slice rotation and selection is simple:
  For a mouse down event, count the number of visible planes, which are "near" the cursor.
  If this number is 2 (one for the window, which currently holds the cursor, one for the
  intersection line of another visible slice), then initiate rotation, else select slices
  near the cursor. If the "LinkPlanes" flag is set, the rotation is applied to the planes
  of all registered SNCs, not only of the one associated with the directly selected plane.

  In contrast to the situation without the SlicesRotator, the SliceNavigationControllers
  are now NOT directly registered as listeners to GlobalInteraction. SlicesRotator is
  registered as a listener and decides whether something should be rotated or whether
  another slice should be selected. In the latter case, a PositionEvent is just forwarded
  to the SliceNavigationController.

  \section mitkSlicesRotator_GeneralizedCase The generalized case: any number of views

  Above section as well as the original implementation of this class assumes that we have
  exactly three 2D vies in our scene. This used to be the standard setup of the MITK
  associated application for a long time.  With custom applications based on MITK it is
  easy to create different situations.  One usual use case would be to have one extra
  render window display the contents of any of the other ones and behave exactly like it
  (could e.g. be used on a second screen).

  In this situation the above assumption "we rotate when there are exactly 2 slices close
  to the cursor" will not hold: since we always have two render windows displaying the
  exact same slice, the number of 2 is the minimum we get. Whenever the user clicks in one
  of those windows and the cursor is close to one of the orthogonal planes, we will get a
  count of 3 or more planes that are "close to the cursor".

  For the class to behave correctly, we actually need to distinguish three separate cases:
   1. the cursor is not close to any orthogonal planes. This should result in slice selection.
   2. the cursor is close to just one orthogonal plane OR multiple which are not distinguishable visually. This should result in rotation.
   3. the cursor is close to multiple orthogonal planes which are rendered as distinguishable lines on the render window. This is the case when we hit the crosshair-center of the view. In this case, we need to also just select slices.

   \section mitkSlicesRotator_Solution Deciding between slice selection and rotation

   The "counting nearby lines in the renderwindow" can also work for the general case
   described above.  Only one details needs to be accounted for: we must not count a line
   when it is identical to another line. I.e. we just count how many visible lines on the
   screen are very close to the cursor. When this number is 1, we rotate, otherwise we let
   the SliceNavigationControllers do their slice selection job.

  \sa SlicesSwiveller
 */
class MITK_CORE_EXPORT SlicesRotator : public SlicesCoordinator
{
  public:

    mitkClassMacro(SlicesRotator, SlicesCoordinator);

    static Pointer New();

    /**
      \brief New Macro with one parameter for creating this object with static New(..) method.

      Needs to be the "slices-rotator" pattern of StateMachine.xml to work as expected.
     **/
    mitkNewMacro1Param(Self, const char*);

    /**
      \brief Callback for modifications in observed SliceNavigationControllers -- forwards to UpdateRotatableSNCs().

      This method is called when an observed SliceNavigationController changes its
      world geometry. The connection is established by calling the other SliceNavigationController's
      method ConnectGeometrySendEvent (or similar).
     */
    virtual void SetGeometry(const itk::EventObject& EventObject);

    /**
      \brief NOT USED by anything open-source. Deprecated. Highly obfuscated code. Use SliceNavigationController::ReorientSlices() instead!
      #Deprecated

     */
    virtual void RotateToPoint( SliceNavigationController *rotationPlaneSNC,
                                SliceNavigationController *rotatedPlaneSNC,
                                const Point3D &point,
                                bool linked = false );

  protected:

    SlicesRotator(const char* machine);
    virtual ~SlicesRotator();

    /**
      \brief Called from SlicesCoordinator after a new controller is added (to internal list m_SliceNavigationControllers).
    */
    virtual void OnSliceControllerAdded(SliceNavigationController* snc);

    /*
      \brief Called from SlicesCoordinator after a new controller is being removed (to internal list m_SliceNavigationControllers).
    */
    virtual void OnSliceControllerRemoved(SliceNavigationController* snc);

    /**
      \brief Check all observed SliceNavigationControllers: remember those that are rotatable in m_RotatableSNCs.
     */
    virtual void UpdateRotatableSNCs();

    // following methods called from superclass ExecuteAction
    bool DoSelectSlice(Action*, const StateEvent*);
    bool DoDecideBetweenRotationAndSliceSelection(Action*, const StateEvent*);
    bool DoStartRotation(Action*, const StateEvent*);
    bool DoEndRotation(Action*, const StateEvent*);
    bool DoRotationStep(Action*, const StateEvent*);

    SNCVector m_RotatableSNCs; /// all SNCs that currently have CreatedWorldGeometries, that can be rotated.
    SNCVector m_SNCsToBeRotated; /// all SNCs that will be rotated (exceptions are the ones parallel to the one being clicked)

    Point3D  m_LastCursorPosition; /// used for calculation of the rotation angle
    Point3D  m_CenterOfRotation; /// used for calculation of the rotation angle

};

} // namespace

#endif


