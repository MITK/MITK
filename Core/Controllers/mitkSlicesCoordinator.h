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


#ifndef SLICESCOORDINATOR_H_HEADER_INCLUDED_C1C55A2F
#define SLICESCOORDINATOR_H_HEADER_INCLUDED_C1C55A2F

#include <mitkStateMachine.h>
#include <vector>

namespace mitk {

class SliceNavigationController;
class Action;
class StateEvent;
 
/**
 * \brief Coordinates a list of SliceNavigationControllers.
 *
 * Each SliceNavigationController can select one slice from a
 * TimeSlicedGeometry. This class (SlicesCoordinator) coordinates several 
 * SliceNavigationControllers to facilitate e.g. rotation of slices. A new 
 * class is needed, because for rotation one has to know an axis of rotation.
 * Such an axis is most easily determined from the "other slices", which are
 * not known by a SliceNavigationController.

 * This class registers itself as a listener to GlobalInteraction and holds a
 * list of SliceNavigationControllers. Any functionality, such as slice
 * rotation, is done in subclasses. This separation is done for the case that 
 * some other multi-slice coordination should be implemented.
 */
class MITK_CORE_EXPORT SlicesCoordinator : public StateMachine
{
public:

  typedef std::vector<SliceNavigationController*> SNCVector;
  
  mitkClassMacro(SlicesCoordinator, StateMachine);
  mitkNewMacro1Param(Self, const char*);   
  

  /** Add to list of managed slices. Check if CreatedWorldGeometry of SNC is
   * managable (i.e. there is basically only one planegeometry) */
  void AddSliceController(SliceNavigationController* snc);

  /** Remove one controller, which is then added as listener to
   * GlobalInteraction */
  void RemoveSliceController(SliceNavigationController* snc);

  /* Reset all Slices to central slice, no rotation */
  // void ResetAllSlices();

  /** Set/Get whether planes should stay linked to each other (by fixing
   * their relative angle) */
  itkSetMacro( LinkPlanes, bool );
  itkGetMacro( LinkPlanes, bool );
  itkBooleanMacro( LinkPlanes );

protected:
  /** \brief Default Constructor */
  SlicesCoordinator(const char* machine);
  
  /** clear list of controllers */
  virtual ~SlicesCoordinator();

  /** for implementation in subclasses */
  virtual void OnSliceControllerAdded(SliceNavigationController* snc);

  /** for implementation in subclasses */
  virtual void OnSliceControllerRemoved(SliceNavigationController* snc);

  /** for implementation in subclasses */
  virtual bool ExecuteAction(Action * action, StateEvent const* stateEvent);

  SNCVector m_SliceNavigationControllers;

  bool m_LinkPlanes;

};

} // namespace 

#endif 


