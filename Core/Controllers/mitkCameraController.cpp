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


#include "mitkCameraController.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"
#include <vtkRenderWindowInteractor.h>
#include "vtkCommand.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

mitk::CameraController::CameraController(const char * type) : BaseController(type), m_Renderer(NULL)
{}


mitk::CameraController::~CameraController()
{}

void mitk::CameraController::Resize(int, int)
{}

void mitk::CameraController::MousePressEvent(mitk::MouseEvent*)
{}

void mitk::CameraController::MouseReleaseEvent(mitk::MouseEvent*)
{}

void mitk::CameraController::MouseMoveEvent(mitk::MouseEvent*)
{}

void mitk::CameraController::KeyPressEvent(mitk::KeyEvent*)
{}
void mitk::CameraController::SetViewToAnterior()
{
  this->SetStandardView(ANTERIOR);
}
void mitk::CameraController::SetViewToPosterior()
{
  this->SetStandardView(POSTERIOR);
}
void mitk::CameraController::SetViewToSinister()
{
  this->SetStandardView(SINISTER);
}
void mitk::CameraController::SetViewToDexter()
{
  this->SetStandardView(DEXTER);
}
void mitk::CameraController::SetViewToCranial()
{
  this->SetStandardView(CRANIAL);
}
void mitk::CameraController::SetViewToCaudal()
{
  this->SetStandardView(CAUDAL);
}

void mitk::CameraController::SetStandardView( mitk::CameraController::StandardView view )
{
  const mitk::VtkPropRenderer* glRenderer = dynamic_cast<const mitk::VtkPropRenderer*>(m_Renderer);
  if (glRenderer)
  {
    vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();
    mitk::DataTree*  tree = dynamic_cast<mitk::DataTree*>(m_Renderer->GetData()->GetTree());
    mitk::BoundingBox::Pointer bb = tree->ComputeBoundingBox(m_Renderer->GetData());

    mitk::Point3D middle =bb->GetCenter();
    vtkRenderer->GetActiveCamera()->SetFocalPoint(middle[0],middle[1],middle[2]);
    switch(view)
    {
    case ANTERIOR:
    case POSTERIOR:
    case SINISTER:
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetViewUp(0,0,1);
      break;
    case CRANIAL:
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetViewUp(0,-1,0);
    }

    switch(view)
    {
    case ANTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0],-100000,middle[2]);
      break;
    case POSTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0],+100000,middle[2]);
      break;
    case SINISTER:
      vtkRenderer->GetActiveCamera()->SetPosition(+100000,middle[1],middle[2]);
      break;
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetPosition(-100000,middle[1],middle[2]);
      break;
    case CRANIAL:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0],middle[1],100000);
      break;
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0],middle[1],-100000);
      break;
    }
    vtkRenderer->ResetCamera();
    
    double *cameraPosition = vtkRenderer->GetActiveCamera()->GetPosition();
    double zoomFactor = 1.0;
    
    switch(view)
    {
    case ANTERIOR:
    case POSTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1] / zoomFactor,cameraPosition[2]);
      break;

    case SINISTER:
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0] / zoomFactor,cameraPosition[1],cameraPosition[2]);
      break;

    case CRANIAL:
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2] / zoomFactor);
      break;
    }

    vtkRenderer->ResetCameraClippingRange();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
