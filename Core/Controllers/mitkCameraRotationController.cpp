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

#include "mitkCameraRotationController.h"

#include <vtkCamera.h>
#include <itkCommand.h>
#include <vtkRenderer.h>
#include "mitkRenderWindow.h"
#include "mitkOpenGLRenderer.h"

mitk::CameraRotationController::CameraRotationController(const char * type)
  : BaseController(type), m_LastStepperValue(180), m_Camera(NULL), m_RenderWindow(NULL)
{
  m_Slice->SetAutoRepeat(true);
  m_Slice->SetSteps(360);
  m_Slice->SetPos(180);

  itk::SimpleMemberCommand<CameraRotationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<CameraRotationController>::New();
  sliceStepperChangedCommand->SetCallbackFunction(this, &CameraRotationController::RotateCamera);

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
}

mitk::CameraRotationController::~CameraRotationController()
{
}

void mitk::CameraRotationController::RotateCamera()
{
  if (!m_Camera)
  {
    this->AcquireCamera();
  }

  if (m_Camera)
  {
    int newStepperValue = m_Slice->GetPos();
    m_Camera->Azimuth( m_LastStepperValue - newStepperValue );
    m_LastStepperValue = newStepperValue;
    const_cast< RenderWindow* >(m_RenderWindow)->RequestUpdate(); // TODO does not work with movie generator!
    //m_MultiWidget->RequestUpdate();
  }
}

void mitk::CameraRotationController::AcquireCamera()
{
  BaseRenderer* renderer = m_RenderWindow->GetRenderer();

  const mitk::OpenGLRenderer *openGLRenderer = dynamic_cast<const mitk::OpenGLRenderer * >( renderer );
  if (openGLRenderer) 
  { 
    // get vtk renderer
    vtkRenderer* vtkrenderer = openGLRenderer->GetVtkRenderer();
    if (vtkrenderer) 
    { 
      // get vtk camera
      vtkCamera* vtkcam = vtkrenderer->GetActiveCamera();
      if (vtkcam) 
      { 
        // vtk smart pointer handling
        if (!m_Camera)
        {
          m_Camera = vtkcam;
          m_Camera->Register( NULL );
        }
        else
        {
          m_Camera->UnRegister( NULL );
          m_Camera = vtkcam;
          m_Camera->Register( NULL );
        }
      }
    }
  }
}

bool mitk::CameraRotationController::ExecuteAction(
  Action*, mitk::StateEvent const*)
{
  return false;
}
