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

#ifndef CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU
#define CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU

#include "mitkCommon.h"
#include "mitkBaseController.h"

class vtkCamera;
class vtkRenderWindow;

namespace mitk {


class CameraRotationController : public BaseController
{
public:
  mitkClassMacro(CameraRotationController,BaseController);
  itkNewMacro(Self);
  mitkNewMacro1Param(Self, const char *);

  void RotateCamera();
  void AcquireCamera();

  void SetRenderWindow(vtkRenderWindow * renWin)
  {
    m_RenderWindow = renWin;
  }

  virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

protected:
  CameraRotationController(const char * type = NULL);
  virtual ~CameraRotationController();

private:
  int m_LastStepperValue;
  vtkCamera* m_Camera;
  vtkRenderWindow* m_RenderWindow;
};

}

#endif
