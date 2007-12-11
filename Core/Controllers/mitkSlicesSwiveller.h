/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 7707 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef SLICESSWIVELLER_H_HEADER_INCLUDED
#define SLICESSWIVELLER_H_HEADER_INCLUDED

#include <mitkSlicesCoordinator.h>
#include <mitkVector.h>

#include <itkEventObject.h>

namespace mitk {

/**
 * \brief Enables arbitrary rotation of visible slices around a swivel point
 * (for sliced geometries).
 * \ingroup NavigationControl
 *
 * This class takes care of several SliceNavigationControllers and handles 
 * slice selection / slice rotation. It is added as listener to 
 * GlobalInteraction by QmitkStdMultiWidget.
 *
 * The SlicesSwiveller class adds the possibility of slice rotation to the
 * "normal" behaviour of SliceNavigationControllers. This additional class
 * is needed, because one has to be aware of several "visible slices" 
 * (selected Geometry2Ds of some SliceNavigationControllers) in order to
 * choose between rotation and slice selection.
 *
 * Rotation is achieved by modifying (rotating) the generated
 * TimeSlicedGeometry of the corresponding SliceNavigationController.
 *
 * With SlicesSwiveller, slice rotation works as follows: the user clicks onto
 * a 2D view (2D plane) and drags the mouse; the relative direction and angle
 * of the dragged mouse movement directly effects the rotation axis and
 * angle. If "LinkPlanes" is set to true, the rotation is applied to the
 * planes of all registered SNCs, not only of the one associated with the
 * plane clicked on.
 *  
 * In contrast to the situation without the SlicesRotator, the
 * SliceNavigationControllers are now not directly registered as listeners to 
 * GlobalInteraction. SlicesRotator is registered as a listener and decides 
 * whether something should be rotated or whether another slice should be 
 * selected. In the latter case, a PositionEvent is just forwarded to the 
 * SliceNavigationController.
 *
 * \sa SlicesRotator
 */
class SlicesSwiveller : public SlicesCoordinator
{
public:

  mitkClassMacro(SlicesSwiveller, SlicesCoordinator);
  
  /// @TODO FIX StateMachine smart pointer handling (or learn about the reason)
  static Pointer New();
  /// @TODO public ONLY because of StateMachine's meddling with m_ReferenceCount
  SlicesSwiveller(const char* machine);

  virtual void SetGeometry(const itk::EventObject& EventObject);

protected:

  // clear list of controllers
  virtual ~SlicesSwiveller();

  // check if the slices of this SliceNavigationController can be rotated (???) Possible
  virtual void OnSliceControllerAdded(SliceNavigationController* snc);

  virtual void OnSliceControllerRemoved(SliceNavigationController* snc);

  virtual void UpdateRelevantSNCs();

  virtual bool ExecuteAction(Action * action, StateEvent const* stateEvent);

  
  /** All SNCs that currently have CreatedWorldGeometries, that can be rotated */
  SNCVector m_RelevantSNCs;
  
  /** SNCs that will be rotated (clicked plane + all relevant others, if linked) */
  SNCVector m_SNCsToBeRotated;

  Point3D m_LastCursorPosition;
  
  Point3D m_CenterOfRotation;
  
  Point2D m_ReferenceCursor;
  
  Vector3D m_RotationPlaneNormal;
  Vector3D m_RotationPlaneXVector;
  Vector3D m_RotationPlaneYVector;

  Vector3D m_PreviousRotationAxis;
  ScalarType m_PreviousRotationAngle;

};

} // namespace 

#endif 

