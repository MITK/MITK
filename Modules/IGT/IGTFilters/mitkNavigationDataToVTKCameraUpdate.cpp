/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToVTKCameraUpdate.h"
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>

mitk::NavigationDataToVTKCameraUpdate::NavigationDataToVTKCameraUpdate()
  : m_Renderer(NULL) , m_vtkRenderer(0) , m_Camera(0)
{
}

mitk::NavigationDataToVTKCameraUpdate::~NavigationDataToVTKCameraUpdate()
{
}


void mitk::NavigationDataToVTKCameraUpdate::GenerateData()
{
  // assert the render
  if(!m_Renderer)
    itkExceptionMacro( << "BaseRender not set");
  const mitk::NavigationData* input 
    = dynamic_cast<const mitk::NavigationData*>(this->GetInput(0));
  if(!input)
    itkExceptionMacro( << "No input specified.");

  if(!input->IsDataValid())
  {
    MITK_WARN("mitk::NavigationDataToVTKCameraUpdate::GenerateData()") 
      << "Input data is invalid. Updating camera not possible.";
    return;
  }

  // get rotation
  vnl_matrix<mitk::ScalarType> vnlRotation( input->GetOrientation().rotation_matrix_transpose() );
  MBI_INFO<< "ND Pos "<< vnlRotation;
  // viewPlaneNormal is rotation[2]         
  double viewPlaneNormal[4];                                                          
  viewPlaneNormal[0] = vnlRotation(2,0);
  viewPlaneNormal[1] = vnlRotation(2,1);
  viewPlaneNormal[2] = vnlRotation(2,2);

  // create focalpoint: focalPoint = P-viewPlaneNormal, 
  vnl_vector<mitk::ScalarType> vnlTranslation = input->GetPosition().GetVnlVector();
  mitk::Point3D focalPoint;
  mitk::FillVector3D(focalPoint
    , vnlTranslation[0] + viewPlaneNormal[0]
    , vnlTranslation[1] + viewPlaneNormal[1]
    , vnlTranslation[2] + viewPlaneNormal[2]);  

  // create ViewUp
  mitk::Point3D viewUp;
  mitk::FillVector3D(viewUp
    , vnlRotation(1,0)
    , vnlRotation(1,1)
    , vnlRotation(1,2) );

  m_Camera->SetPosition(vnlTranslation[0], vnlTranslation[1], vnlTranslation[2]);
  m_Camera->SetFocalPoint(focalPoint[0], focalPoint[1], focalPoint[2]);
  m_Camera->SetViewUp(viewUp[0], viewUp[1], viewUp[2]);
  
  m_Camera->ComputeViewPlaneNormal ();
  
  m_vtkRenderer->ResetCameraClippingRange();
}

void mitk::NavigationDataToVTKCameraUpdate::SetRenderer(mitk::BaseRenderer*  renderer)
{
  if(m_Renderer == renderer)
    return;

  m_Renderer = renderer;

  if(renderer)
  {
    m_vtkRenderer = m_Renderer->GetVtkRenderer();
    m_Camera = m_vtkRenderer->GetActiveCamera();    
  }
  else
  {
    m_vtkRenderer = 0;
    m_Camera = 0;
  }
  this->Modified();
}

const mitk::BaseRenderer* mitk::NavigationDataToVTKCameraUpdate::GetRenderer()
{
  return m_Renderer;
}
