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
