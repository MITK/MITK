/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkCameraVisualization.h"

#include "vtkCamera.h"

#include "vtkSphereSource.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkSurface.h"

#include "mitkDataStorage.h"

namespace mitk{

  CameraVisualization::CameraVisualization()
  {
    // initialize members
    m_Renderer = NULL;
    m_DirectionOfProjectionInToolCoordinates[0] = 0;
    m_DirectionOfProjectionInToolCoordinates[1] = 0;
    m_DirectionOfProjectionInToolCoordinates[2] = -1;
    m_ViewUpInToolCoordinates[0] = -1;
    m_ViewUpInToolCoordinates[1] = 0;
    m_ViewUpInToolCoordinates[2] = 0;
    m_FocalLength = 10;
  }

  CameraVisualization::~CameraVisualization()
  {

  }

  void CameraVisualization::Update()
  {
    // check if renderer was set
    if (m_Renderer)
    {
      const NavigationData* navigationData = this->GetInput();
      // get position from NavigationData
      Point3D cameraPosition = navigationData->GetPosition();
      // temp
      double* fP = m_Renderer->GetVtkRenderer()->GetActiveCamera()->GetFocalPoint();
      Point3D fPoint;
      fPoint[0] = fP[0];
      fPoint[1] = fP[1];
      fPoint[2] = fP[2];
      float length = cameraPosition.EuclideanDistanceTo(fPoint);
      // end temp
      // get orientation from NavigationData
      Quaternion cameraOrientationQuaternion = navigationData->GetOrientation();
      vnl_matrix_fixed<ScalarType,3,3> cameraOrientationMatrix = cameraOrientationQuaternion.rotation_matrix_transpose();
      itk::Matrix<ScalarType,3,3> itkRotationMatrix(cameraOrientationMatrix);
      // compute current direction of projection and focal point
      Vector3D directionOfProjection = itkRotationMatrix*m_DirectionOfProjectionInToolCoordinates;
      directionOfProjection.Normalize();
      Point3D focalPoint = cameraPosition + m_FocalLength*directionOfProjection;
      // compute current view up vector
      Vector3D viewUp = itkRotationMatrix*m_ViewUpInToolCoordinates;

      //mitk::DataTreeNode::Pointer cameraPositionSphereNode = mitk::DataTreeNode::New();
      //cameraPositionSphereNode->SetProperty("color",mitk::ColorProperty::New(1,0,0));
      //vtkSphereSource* sphere = vtkSphereSource::New();
      //sphere->SetCenter(cameraPosition[0],cameraPosition[1],cameraPosition[2]);
      //sphere->SetRadius(10);
      //mitk::Surface::Pointer sphereSurface = mitk::Surface::New();
      //sphereSurface->SetVtkPolyData(sphere->GetOutput());
      //cameraPositionSphereNode->SetData(sphereSurface);
      //mitk::DataStorage::GetInstance()->Add(cameraPositionSphereNode);

      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2]);
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(focalPoint[0],focalPoint[1],focalPoint[2]);
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetViewUp(viewUp[0],viewUp[1],viewUp[2]);

      m_Renderer->GetVtkRenderer()->ResetCameraClippingRange();

      std::cout<<"Camera Position: "<<cameraPosition<<std::endl;
      std::cout<<"Direction of Projection: "<<directionOfProjection<<std::endl;
      std::cout<<"Focal Point: "<<focalPoint<<std::endl;
      std::cout<<"View Up: "<<viewUp<<std::endl;
      std::cout<<"Orientation: "<<itkRotationMatrix<<std::endl<<std::endl;
    }
    else
    {
      itkExceptionMacro(<< "Renderer was not properly set");
    }
  }

  void CameraVisualization::SetRenderer(VtkPropRenderer::Pointer renderer)
  {
    m_Renderer = renderer;
  }

  const BaseRenderer* CameraVisualization::GetRenderer()
  {
    return m_Renderer;
  }

  void CameraVisualization::SetParameters( const mitk::PropertyList* p )
  {
    if (p == NULL)
      return;
    mitk::Vector3D doP;
    if (p->GetPropertyValue<mitk::Vector3D>("CameraVisualization_DirectionOfProjectionInToolCoordinates", doP) == true)  // search for Offset parameter
      this->SetDirectionOfProjectionInToolCoordinates(doP);   // apply if found;
    mitk::Vector3D vUp;
    if (p->GetPropertyValue<mitk::Vector3D>("CameraVisualization_ViewUpInToolCoordinates", vUp) == true)  // search for Offset parameter
      this->SetViewUpInToolCoordinates(vUp);   // apply if found;
    float fL;
    if (p->GetPropertyValue<float>("CameraVisualization_FocalLength", fL) == true)  // search for Offset parameter
      this->SetFocalLength(fL);   // apply if found;
  }

  mitk::PropertyList::ConstPointer CameraVisualization::GetParameters() const
  {
    mitk::PropertyList::Pointer p = mitk::PropertyList::New();
    p->SetProperty("CameraVisualization_DirectionOfProjectionInToolCoordinates", mitk::Vector3DProperty::New(this->GetDirectionOfProjectionInToolCoordinates()));  // store Offset parameter
    p->SetProperty("CameraVisualization_ViewUpInToolCoordinates", mitk::Vector3DProperty::New(this->GetViewUpInToolCoordinates()));  // store Offset parameter
    p->SetProperty("CameraVisualization_FocalLength", mitk::Vector3DProperty::New(this->GetFocalLength()));  // store Offset parameter
    return mitk::PropertyList::ConstPointer(p);
  }
}
