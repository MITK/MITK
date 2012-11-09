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


#include <mitkKeyEvent.h>
#include <mitkWheelEvent.h>
#include <mitkDisplayPositionEvent.h>

#include "vtkRenderWindowInteractor.h"

//##Documentation
  //## @brief Generates MITK events from VTK
  //##
  //## This class is the NON-QT dependent pandon to QmitkEventAdapter.
  //## It provides static functions to set up MITK events from VTK source data
  //## @ingroup Interaction

namespace mitk
{

class BaseRenderer;

class MITK_EXPORT VtkEventAdapter
{
public:
  static mitk::MouseEvent AdaptMouseEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::WheelEvent AdaptWheelEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);
  static mitk::KeyEvent AdaptKeyEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi);

  static std::map<BaseRenderer* ,int> buttonStateMap;
};

}

#endif /*QMITKEVENTADAPTER_H_*/
