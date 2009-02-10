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

namespace mitk{

  CameraVisualization::CameraVisualization()
  {

  }

  CameraVisualization::~CameraVisualization()
  {

  }

  void mitk::CameraVisualization::GenerateData()
  {
    // check if renderer was set
    if (m_Renderer)
    {
      //const NavigationData* navigationData = this->GetInput();
      //// get position from NavigationData
      //Point3D cameraPosition = navigationData->GetPosition();
      //// get orientation from NavigationData
      //Quaternion cameraOrientationQuaternion = navigationData->GetOrientation();
      //vnl_matrix_fixed<ScalarType,3,3> cameraOrientationMatrix = cameraOrientationQuaternion.rotation_matrix_transpose();
      //m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2]);
    }
    else
    {
      itkExceptionMacro(<< "Renderer was not properly set");
    }
  }

}