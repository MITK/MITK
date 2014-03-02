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

#ifndef MITKTDMOUSEVTKCAMERACONTROLLER_H_HEADER_INCLUDED
#define MITKTDMOUSEVTKCAMERACONTROLLER_H_HEADER_INCLUDED

#include "mitkCameraController.h"


class vtkRenderWindow;

namespace mitk {

//##Documentation
//## @brief vtk-based camera controller
//## @ingroup NavigationControl
class TDMouseVtkCameraController : public CameraController
{
public:
  mitkClassMacro(TDMouseVtkCameraController, CameraController);
  itkNewMacro(Self);

  virtual bool OnTDMouseEvent(mitk::Action* a, const mitk::StateEvent* e);
  virtual bool OnTDMouseKeyDown(mitk::Action* a, const mitk::StateEvent* e);

protected:
  /**
  * @brief Default Constructor
  **/
  TDMouseVtkCameraController();

  /**
  * @brief Default Destructor
  **/
  virtual ~TDMouseVtkCameraController();

  bool m_ClippingRangeIsSet;


};

} // namespace mitk

#endif /* MITKTDMOUSEVTKCAMERACONTROLLER_H_HEADER_INCLUDED */
