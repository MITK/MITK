/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

/** \brief Enables rotation of visible slices.

  This class takes care of several SliceNavigationControllers and handles 
  slice selection / slice rotation.
  It is added as listener to GlobalInteraction by QmitkStdMultiWidget.

  The rule to choose between slice rotation and selection is easy:
  - For a mouse down event, count the number of visible planes, which
    are "near" the cursor. If this number equals 2 (one for the window, which
    currently holds the cursor, one for the intersection line of another visible slice), 
    then initiate rotation, else select slices near the cursor.
  */
class SlicesRotator : public SlicesCoordinator
{
  public:

    mitkClassMacro(SlicesRotator, SlicesCoordinator);
   
    /// @TODO FIX StateMachine smart pointer handling (or learn about the reason)
    static Pointer New();
    /// @TODO public ONLY because of StateMachine's meddling with m_ReferenceCount
    SlicesRotator(const char* machine);

    virtual void SetGeometry(const itk::EventObject& EventObject);
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

