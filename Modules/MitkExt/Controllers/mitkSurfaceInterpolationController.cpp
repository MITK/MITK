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

#include "mitkSurfaceInterpolationController.h"

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  //for (unsigned int i = 0; i < m_PositionList.size(); i++)
  //{
  //  delete m_PositionList.at(i);
  //}
}

mitk::SurfaceInterpolationController* mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new SurfaceInterpolationController();
  }

  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour /*,PlanePositionOperation*/)
{
  //Now the extracted contour must be reduced and the normals must be calculated in order to perform 3D Interpolation
  if (m_ContourList.size() != 0)
  {
    //FIRST: check if there is already a surface that exists at this position -> TODO
    for (unsigned int i = 0; i < m_ContourList.size(); i++)
    {
      m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    }
  }
  else
  {
    //Soll das alles jetzt schon gemacht werden? Information für Reduktion neuer Konturen geht verloren!
    ContourPositionPair newData;
    m_ReduceFilter->SetInput(newContour);
    m_ReduceFilter->Update();
    m_NormalsFilter->SetInput(m_ReduceFilter->GetOutput());
    newData.contour = m_NormalsFilter->GetOutput();
    m_ContourList.push_back(newData);
  }
  //m_ReduceFilter->RemoveInputs();
  //m_ReduceFilter->SetInput(i);
  
  //Hier Reduce und Normals berechnen
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::Interpolate()
{
  return 0;
}