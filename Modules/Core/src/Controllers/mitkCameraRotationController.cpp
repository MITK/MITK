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

#include <vtkCamera.h>
#include <itkCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"


mitk::CameraRotationController::CameraRotationController()
  : BaseController(), m_LastStepperValue(180), m_Camera(NULL), m_RenderWindow(NULL)
{
  m_Slice->SetAutoRepeat(true);
  m_Slice->SetSteps(360);
  m_Slice->SetPos(180);

  m_ElevationSlice = Stepper::New();
  m_ElevationSlice->SetAutoRepeat(false);
  m_ElevationSlice->SetSteps(180);
  m_ElevationSlice->SetPos(90);

  itk::SimpleMemberCommand<CameraRotationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<CameraRotationController>::New();
  sliceStepperChangedCommand->SetCallbackFunction(this, &CameraRotationController::RotateCamera);

  itk::SimpleMemberCommand<CameraRotationController>::Pointer elevateSliceStepperChangedCommand;
  elevateSliceStepperChangedCommand = itk::SimpleMemberCommand<CameraRotationController>::New();
  elevateSliceStepperChangedCommand->SetCallbackFunction(this, &CameraRotationController::Elevate);

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
  m_ElevationSlice->AddObserver(itk::ModifiedEvent(), elevateSliceStepperChangedCommand);
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
    //const_cast< RenderWindow* >(m_RenderWindow)->RequestUpdate(); // TODO does not work with movie generator!
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow);
    //m_MultiWidget->RequestUpdate();
  }
}

void mitk::CameraRotationController::Elevate()
{
  if (!m_Camera)
  {
    this->AcquireCamera();
  }
  if (m_Camera)
  {
    int newStepperValue = m_ElevationSlice->GetPos();
    if (newStepperValue == 0) return;
    m_Camera->Elevation( m_ElevateLastStepperValue - newStepperValue );
    m_ElevateLastStepperValue = newStepperValue;
    //const_cast< RenderWindow* >(m_RenderWindow)->RequestUpdate(); // TODO does not work with movie generator!
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow);
    //m_MultiWidget->RequestUpdate();
  }
}

void mitk::CameraRotationController::RotateToAngle(double angle)
{
  if (!m_Camera)
  {
    this->AcquireCamera();
  }
  if (m_Camera)
  {
    m_Camera->SetRoll(m_Camera->GetRoll() + angle);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow);
  }
}

void mitk::CameraRotationController::AcquireCamera()
{
  BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(m_RenderWindow);

  const mitk::VtkPropRenderer *propRenderer = dynamic_cast<const mitk::VtkPropRenderer * >( renderer );
  if (propRenderer)
  {
    // get vtk renderer
    vtkRenderer* vtkrenderer = propRenderer->GetVtkRenderer();
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
