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


#ifndef SLICESROTATOR_H_HEADER_INCLUDED_C1C55A2F
#define SLICESROTATOR_H_HEADER_INCLUDED_C1C55A2F

#include <mitkSlicesCoordinator.h>
#include <itkEventObject.h>
#include <mitkVector.h>

namespace mitk {

/**
 * \brief Enables rotation of visible slices (for sliced geometries).
 * \ingroup NavigationControl
 *
 * This class takes care of several SliceNavigationControllers and handles 
 * slice selection / slice rotation. It is added as listener to 
 * GlobalInteraction by QmitkStdMultiWidget.
 *
 * The SlicesRotator class adds the possibility of slice rotation to the
 * "normal" behaviour of SliceNavigationControllers. This additional class
 * is needed, because one has to be aware of several "visible slices" 
 * (selected Geometry2Ds of some SliceNavigationControllers) in order to
 * choose between rotation and slice selection.
 *
 * Rotation is achieved by modifying (rotating) the generated
 * TimeSlicedGeometry of the corresponding SliceNavigationController.
 *
 * With SlicesRotator, the rule to choose between slice rotation and 
 * selection is simple: For a mouse down event, count the number of visible
 * planes, which are "near" the cursor. If this number equals 2 (one for the 
 * window, which currently holds the cursor, one for the intersection line of
 * another visible slice), then initiate rotation, else select slices near 
 * the cursor. If "LinkPlanes" is set to true, the rotation is applied to the
 * planes of all registered SNCs, not only of the one associated with the
 * directly selected plane.
 *  
 * In contrast to the situation without the SlicesRotator, the
 * SliceNavigationControllers are now not directly registered as listeners to 
 * GlobalInteraction. SlicesRotator is registered as a listener and decides 
 * whether something should be rotated or whether another slice should be 
 * selected. In the latter case, a PositionEvent is just forwarded to the 
 * SliceNavigationController.
 *
 * \sa SlicesSwiveller
 */
class MITK_CORE_EXPORT SlicesRotator : public SlicesCoordinator
{
public:

  mitkClassMacro(SlicesRotator, SlicesCoordinator);
  
  /// @TODO FIX StateMachine smart pointer handling (or learn about the reason)
  static Pointer New();
  /// @TODO public ONLY because of StateMachine's meddling with m_ReferenceCount
  SlicesRotator(const char* machine);

  virtual void SetGeometry(const itk::EventObject& EventObject);

  virtual void RotateToPoint( SliceNavigationController *rotationPlaneSNC,
    SliceNavigationController *rotatedPlaneSNC,
    const Point3D &point, bool linked = false );

protected:

  // clear list of controllers
  virtual ~SlicesRotator();

  // check if the slices of this SliceNavigationController can be rotated (???) Possible
  virtual void OnSliceControllerAdded(SliceNavigationController* snc);

  virtual void OnSliceControllerRemoved(SliceNavigationController* snc);

  virtual void UpdateRelevantSNCs();

  virtual bool ExecuteAction(Action * action, StateEvent const* stateEvent);

  SNCVector m_RelevantSNCs; /// all SNCs that currently have CreatedWorldGeometries, that can be rotated.
  SNCVector m_SNCsToBeRotated; /// all SNCs that will be rotated

  Point3D  m_LastCursorPosition;
  Point3D  m_CenterOfRotation;

};

} // namespace 

#endif 


