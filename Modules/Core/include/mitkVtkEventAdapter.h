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

#ifndef MITKVTKEVENTADAPTER_H_
#define MITKVTKEVENTADAPTER_H_


#include "mitkMousePressEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkInteractionKeyEvent.h"


#include "vtkRenderWindowInteractor.h"

//##Documentation
  //## @brief Generates MITK events from VTK
  //##
  //## This class is the NON-QT dependent pendant to QmitkEventAdapter.
  //## It provides static functions to set up MITK events from VTK source data
  //##
  //## @ingroup Interaction

namespace mitk
{

class BaseRenderer;

class MITKCORE_EXPORT VtkEventAdapter
{
public:

  /** New events compatible with the revised interaction scheme */

  static mitk::MousePressEvent::Pointer AdaptMousePressEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::MouseMoveEvent::Pointer AdaptMouseMoveEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::MouseReleaseEvent::Pointer AdaptMouseReleaseEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::MouseWheelEvent::Pointer AdaptMouseWheelEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::InteractionKeyEvent::Pointer AdaptInteractionKeyEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);

  /**
   * Vtk events do not provide a button state for MouseMove events, therefore this map is used to keep a record of the
   * button state, which then can be used to provide information for MITK MouseMove events.
   */
  static std::map<BaseRenderer* ,int> buttonStateMap;
};

}

#endif /*QMITKEVENTADAPTER_H_*/
