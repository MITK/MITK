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


#ifndef SLICESCOORDINATOR_H_HEADER_INCLUDED_C1C55A2F
#define SLICESCOORDINATOR_H_HEADER_INCLUDED_C1C55A2F

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <vector>

namespace mitk {

class SliceNavigationController;
class Action;
class StateEvent;

#pragma GCC visibility push(default)
itkEventMacro( SliceRotationEvent, itk::AnyEvent);
#pragma GCC visibility pop

/**
 * \brief Coordinates a list of SliceNavigationControllers.
 *
 * Each SliceNavigationController can select one slice from a
 * TimeGeometry. This class (SlicesCoordinator) coordinates several
 * SliceNavigationControllers to facilitate e.g. rotation of slices. A new
 * class is needed, because for rotation one has to know an axis of rotation.
 * Such an axis is most easily determined from the "other slices", which are
 * not known by a SliceNavigationController.
 */
class MITKCORE_EXPORT SlicesCoordinator : public itk::Object
{
public:
  mitkClassMacroItkParent(SlicesCoordinator,itk::Object);
  itkFactorylessNewMacro(Self)




  typedef std::vector<SliceNavigationController*> SNCVector;
  /** Add to list of managed slices. Check if CreatedWorldGeometry of SNC is
   * managable (i.e. there is basically only one planegeometry) */
  void AddSliceController(SliceNavigationController* snc);

  /** Remove one controller from the internal list */
  void RemoveSliceController(SliceNavigationController* snc);

  /* Reset all Slices to central slice, no rotation */
  // void ResetAllSlices();

  /** Set/Get whether planes should stay linked to each other (by fixing
   * their relative angle) */
  itkSetMacro( LinkPlanes, bool );
  itkGetMacro( LinkPlanes, bool );
  itkBooleanMacro( LinkPlanes );

  /** \brief Resets the mouse cursor (if modified by the SlicesCoordinator)
   * to its original state.
   *
   * Should be used by subclasses and from external application instead
   * of using QmitkApplicationCursor directly to avoid conflicts. */
  void ResetMouseCursor();


protected:
  /** \brief Default Constructor */
  SlicesCoordinator();

  /** clear list of controllers */
  virtual ~SlicesCoordinator();

  /** \brief Sets the specified mouse cursor.
   *
   * Use this in subclasses instead of using QmitkApplicationCursor directly.
   */
  void SetMouseCursor( const char *xpm[], int hotspotX, int hotspotY );

  /** for implementation in subclasses */
  virtual void OnSliceControllerAdded(SliceNavigationController* snc);

  /** for implementation in subclasses */
  virtual void OnSliceControllerRemoved(SliceNavigationController* snc);

  SNCVector m_SliceNavigationControllers;

  bool m_LinkPlanes;

  bool m_MouseCursorSet;

};

} // namespace

#endif


