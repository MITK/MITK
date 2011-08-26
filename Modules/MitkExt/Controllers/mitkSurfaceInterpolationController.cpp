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
  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  m_PolyData->SetPoints(vtkPoints::New());
  //m_PolyData->SetPolys(vtkCellArray::New());
  //m_Contours->SetVtkPolyData(m_PolyData);

  m_PolyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
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

  if(m_PolyData->GetPoints()->GetNumberOfPoints() == 0)
  {
    //Hier noch überprüfen, ob contour schon existiert
    m_PolyData = newContour->GetVtkPolyData();
    m_PolyDataAppender->AddInput(newContour->GetVtkPolyData());
  }
  else
  {
    //Hier noch überprüfen, ob contour schon existiert
    m_PolyDataAppender->AddInput(newContour->GetVtkPolyData());
    m_PolyDataAppender->Update();
    m_PolyData = m_PolyDataAppender->GetOutput();
  }
  m_Contours->SetVtkPolyData(m_PolyData);



  //m_Contours->SetVtkPolyData(newContour->GetVtkPolyData(),m_ContourList.size());
    ContourPositionPair newData;
    newData.contour = newContour;
    m_ContourList.push_back(newData);
    this->Modified();
  /*  m_Modified = true;*/

}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::Interpolate()
{
  //MITK_INFO<<"Modified: "<<m_Modified;
  if (m_ContourList.size() < 2 /*|| !m_Modified*/)
    return 0;  
  
  m_ReduceFilter->SetMinSpacing(m_MinSpacing);
  m_ReduceFilter->SetMaxSpacing(m_MaxSpacing);
  m_InterpolateSurfaceFilter->SetDistanceImageVolume(50000);

  for (unsigned int i = 0; i < m_ContourList.size(); i++)
  {
    m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
    m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }

  //m_ReduceFilter->SetMinSpacing(m_MinSpacing);
  //m_ReduceFilter->SetMaxSpacing(m_MaxSpacing);
  //m_ReduceFilter->SetReductionType(mitk::ReduceContourSetFilter::NTH_POINT);
  //m_ReduceFilter->Modified();
  //m_ReduceFilter->Update();

  //MITK_INFO<<"Inputs "<<m_ReduceFilter->GetNumberOfInputs();

  //for (unsigned int i = 0; i < m_ContourList.size(); i++)
  //{
  //  //m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
  //  m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
  //  //m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  //}

  //m_NormalsFilter->Update();

  //for (unsigned int i = 0; i < m_ContourList.size(); i++)
  //{
  //  //m_ReduceFilter->SetInput(i,m_ContourList.at(i).contour);
  //  //m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
  //  m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  //}

//  m_InterpolateSurfaceFilter->SetDistanceImageVolume(50000);
  m_InterpolateSurfaceFilter->Update();
  //Surface::Pointer interpolatedSurface = m_InterpolateSurfaceFilter->GetOutput();

  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();
  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput(distanceImage->GetVtkImageData());
  mcFilter->SetValue(0,0);
  vtkSmartPointer<vtkPolyData> pd = mcFilter->GetOutput();

  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(pd);
  Point3D origin = distanceImage->GetGeometry()->GetOrigin();
  distanceImage->GetGeometry()->ChangeImageGeometryConsideringOriginOffset(false);
  //MITK_INFO<<"ImageGeometry: "<<distanceImage->GetGeometry()->GetImageGeometry();
  //MITK_INFO<<"Origin: "<<origin;
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

mitk::Surface* mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}