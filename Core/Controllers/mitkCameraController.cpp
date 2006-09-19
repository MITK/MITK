/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "mitkOpenGLRenderer.h"
#include "mitkRenderingManager.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"

//##ModelId=3E3AE7440312
mitk::CameraController::CameraController(const char * type) : BaseController(type)
{}


//##ModelId=3E3AE75803CF
mitk::CameraController::~CameraController()
{}

//##ModelId=3E6D5DD400A3
void mitk::CameraController::Resize(int, int)
{}

//##ModelId=3E6D5DD400C1
void mitk::CameraController::MousePressEvent(mitk::MouseEvent*)
{}

//##ModelId=3E6D5DD400D5
void mitk::CameraController::MouseReleaseEvent(mitk::MouseEvent*)
{}

//##ModelId=3E6D5DD400F3
void mitk::CameraController::MouseMoveEvent(mitk::MouseEvent*)
{}

//##ModelId=3E6D5DD40107
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
  const mitk::OpenGLRenderer* glRenderer = dynamic_cast<const mitk::OpenGLRenderer*>(m_Renderer);
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
    mitk::Point3D min =bb->GetMinimum();
    mitk::Point3D max =bb->GetMaximum();
    vtkRenderer->ResetCamera(min[0],max[0],min[1],max[1],min[2],max[2]);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
