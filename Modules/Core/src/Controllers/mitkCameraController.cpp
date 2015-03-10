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


#include "mitkCameraController.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"
#include <vtkRenderWindowInteractor.h>
#include "vtkCommand.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

mitk::CameraController::CameraController(const char * type) : BaseController(type), m_Renderer(NULL), m_ZoomFactor(1.0)
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
  if (glRenderer == NULL)
    return;
  vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();
  assert (vtkRenderer);

  mitk::BoundingBox::Pointer bb;
  mitk::DataStorage* ds = m_Renderer->GetDataStorage();
  if (ds != NULL)
    bb = ds->ComputeBoundingBox();
  else
    return;

  if(m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D)
  {
    //set up the view for the 3D render window. The views for 2D are set up in the mitkVtkPropRenderer
    mitk::Point3D middle = bb->GetCenter();
    vtkRenderer->GetActiveCamera()->SetFocalPoint(middle[0], middle[1], middle[2]);
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
      break;
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
    switch(view)
    {
    case ANTERIOR:
    case POSTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1] / m_ZoomFactor,cameraPosition[2]);
      break;

    case SINISTER:
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0] / m_ZoomFactor,cameraPosition[1],cameraPosition[2]);
      break;

    case CRANIAL:
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2] / m_ZoomFactor);
      break;
    }
    vtkRenderer->ResetCameraClippingRange();
  }

  mitk::RenderingManager* rm = m_Renderer->GetRenderingManager();
  rm->RequestUpdateAll();
}
