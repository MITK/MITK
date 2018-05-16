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

#include "mitkCameraRotationController.h"

#include <itkCommand.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include "mitkRenderingManager.h"
#include "mitkVtkPropRenderer.h"

mitk::CameraRotationController::CameraRotationController()
  : BaseController(), m_LastStepperValue(180), m_Camera(nullptr), m_RenderWindow(nullptr)
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
    m_Camera->Azimuth(m_LastStepperValue - newStepperValue);
    m_LastStepperValue = newStepperValue;
    // const_cast< RenderWindow* >(m_RenderWindow)->RequestUpdate(); // TODO does not work with movie generator!
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow);
    // m_MultiWidget->RequestUpdate();
  }
}

void mitk::CameraRotationController::AcquireCamera()
{
  BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(m_RenderWindow);

  const auto *propRenderer = dynamic_cast<const mitk::VtkPropRenderer *>(renderer);
  if (propRenderer)
  {
    // get vtk renderer
    vtkRenderer *vtkrenderer = propRenderer->GetVtkRenderer();
    if (vtkrenderer)
    {
      // get vtk camera
      vtkCamera *vtkcam = vtkrenderer->GetActiveCamera();
      if (vtkcam)
      {
        // vtk smart pointer handling
        if (!m_Camera)
        {
          m_Camera = vtkcam;
          m_Camera->Register(nullptr);
        }
        else
        {
          m_Camera->UnRegister(nullptr);
          m_Camera = vtkcam;
          m_Camera->Register(nullptr);
        }
      }
    }
  }
}
