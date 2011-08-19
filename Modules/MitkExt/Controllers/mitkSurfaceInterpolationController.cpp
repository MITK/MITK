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
#include "vtkMarchingCubes.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
//:m_Modified (false)
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();
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


    /*Check whether a position already exists

    for (unsigned int i = 0; i < m_ContourList.size(); i++)
    {
      m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    }
    */
    ContourPositionPair newData;
    newData.contour = newContour;
    m_ContourList.push_back(newData);
    this->Modified();
  /*  m_Modified = true;*/
  //m_ReduceFilter->RemoveInputs();
  //m_ReduceFilter->SetInput(i);
  
  //Hier Reduce und Normals berechnen
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::Interpolate()
{
  //MITK_INFO<<"Modified: "<<m_Modified;
  if (m_ContourList.size() < 2 /*|| !m_Modified*/)
    return 0;  
  
  for (unsigned int i = 0; i < m_ContourList.size(); i++)
  {
    m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    //m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));oooooo
    //m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }

  m_ReduceFilter->SetMinSpacing(m_MinSpacing);
  m_ReduceFilter->SetMaxSpacing(m_MaxSpacing);
  //m_ReduceFilter->SetReductionType(mitk::ReduceContourSetFilter::NTH_POINT);
  m_ReduceFilter->Update();

  MITK_INFO<<"Inputs "<<m_ReduceFilter->GetNumberOfInputs();

  for (unsigned int i = 0; i < m_ContourList.size(); i++)
  {
    //m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
    //m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }

  m_NormalsFilter->Update();

  for (unsigned int i = 0; i < m_ContourList.size(); i++)
  {
    //m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    //m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }

  m_InterpolateSurfaceFilter->SetDistanceImageVolume(50000);
  m_InterpolateSurfaceFilter->Update();
  //Surface::Pointer interpolatedSurface = m_InterpolateSurfaceFilter->GetOutput();

  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();
  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput(distanceImage->GetVtkImageData());
  mcFilter->SetValue(0,0);
  vtkSmartPointer<vtkPolyData> pd = mcFilter->GetOutput();

  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(pd);
  surface->GetGeometry()->SetOrigin(distanceImage->GetGeometry()->GetOrigin());
  //for (unsigned int i = 0; i < m_ContourList.size(); i++)
  //{
  //  m_ReduceFilter->RemoveOutput(i);
  //  m_NormalsFilter->RemoveOutput(i);
  //  m_InterpolateSurfaceFilter->RemoveOutput(i);
  //}

  //m_Modified = false;
  return surface;
  
}

//bool mitk::SurfaceInterpolationController::DataSetHasChanged()
//{
// /* return m_Modified;*/
//}