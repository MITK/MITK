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


#ifndef BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3
#define BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3

#include "mitkCommon.h"
#include "mitkStepper.h"
#include "mitkStateMachine.h"
#include <itkObjectFactory.h>

namespace mitk {

class BaseRenderer;

//##Documentation
//## @brief Baseclass for renderer slice-/camera-control
//##
//## Tells the render (subclass of BaseRenderer) which slice (subclass
//## SliceNavigationController) or from which direction (subclass
//## CameraController) it has to render. Contains two Stepper for stepping
//## through the slices or through different camera views (e.g., for the
//## creation of a movie around the data), respectively, and through time, if
//## there is 3D+t data.
//## @note not yet implemented
//## @ingroup NavigationControl
class BaseController : public StateMachine
{
public:
  /** Standard class typedefs. */
  mitkClassMacro(BaseController, StateMachine);
  itkNewMacro(Self);

  /** Method for creation through ::New */
  mitkNewMacro1Param(Self, const char *);

  //##Documentation
  //## @brief Get the Stepper through the slices
  mitk::Stepper* GetSlice();

  //##Documentation
  //## @brief Get the Stepper through the time
  mitk::Stepper* GetTime();

protected:
  /**
  * @brief Default Constructor
  **/
  BaseController(const char * type = NULL);

  /**
  * @brief Default Destructor
  **/
  virtual ~BaseController();

  //## @brief Stepper through the time
  Stepper::Pointer m_Time;
  //## @brief Stepper through the slices
  Stepper::Pointer m_Slice;
  
  unsigned long m_LastUpdateTime;
};

} // namespace mitk

#endif /* BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3 */
