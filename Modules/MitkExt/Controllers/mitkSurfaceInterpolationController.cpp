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
: m_CurrentContourListID (0)
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  m_PolyData->SetPoints(vtkPoints::New());

  m_InterpolationResult = 0;
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  for (unsigned int i = 0; i < m_ListOfContourLists.size(); ++i)
  {
      for (unsigned int j = 0; j < m_ListOfContourLists.at(i).size(); ++j)
      {
          delete(m_ListOfContourLists.at(i).at(j).position);
      }
  }
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

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour ,RestorePlanePositionOperation* op)
{
  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform = op->GetTransform();

  mitk::Vector3D direction = op->GetDirectionVector();
  int pos (-1);

  for (unsigned int i = 0; i < m_ListOfContourLists.at(m_CurrentContourListID).size(); i++)
  {
      itk::Matrix<float> diffM = transform->GetMatrix()-m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetTransform()->GetMatrix();
      bool isSameMatrix(true);
      for (unsigned int j = 0; j < 3; j++)
      {
        if (fabs(diffM[j][0]) > 0.0001 && fabs(diffM[j][1]) > 0.0001 && fabs(diffM[j][2]) > 0.0001)
        {
          isSameMatrix = false;
          break;
        }
      }
      itk::Vector<float> diffV = m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetTransform()->GetOffset()-transform->GetOffset();
      if ( isSameMatrix && m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetPos() == op->GetPos() && (fabs(diffV[0]) < 0.0001 && fabs(diffV[1]) < 0.0001 && fabs(diffV[2]) < 0.0001) )
      {
        pos = i;
        break;
      }
    
  }

  if (pos == -1)
  {
    //MITK_INFO<<"New Contour";
    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, op->GetWidth(), 
      op->GetHeight(), op->GetSpacing(), op->GetPos(), direction, transform);
    ContourPositionPair newData;
    newData.contour = newContour;
    newData.position = newOp;

    m_ReduceFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), newContour);
    m_NormalsFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), m_ReduceFilter->GetOutput(m_ListOfContourLists.at(m_CurrentContourListID).size()));
    m_InterpolateSurfaceFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), m_NormalsFilter->GetOutput(m_ListOfContourLists.at(m_CurrentContourListID).size()));
    m_ListOfContourLists.at(m_CurrentContourListID).push_back(newData);
  }
  else
  {
    //MITK_INFO<<"Modified Contour";
    m_ListOfContourLists.at(m_CurrentContourListID).at(pos).contour = newContour;
    m_ReduceFilter->SetInput(pos, newContour);
    m_NormalsFilter->SetInput(pos, m_ReduceFilter->GetOutput(pos));
    m_InterpolateSurfaceFilter->SetInput(pos, m_NormalsFilter->GetOutput(pos));
  }

  this->Modified();
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_ListOfContourLists.at(m_CurrentContourListID).size() < 2)
    return;

  m_InterpolateSurfaceFilter->Update();

  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();

  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput(distanceImage->GetVtkImageData());
  mcFilter->SetValue(0,0);
  mcFilter->Update();

  m_InterpolationResult = 0;
  m_InterpolationResult = mitk::Surface::New();
  m_InterpolationResult->SetVtkPolyData(mcFilter->GetOutput());
  m_InterpolationResult->GetGeometry()->SetOrigin(distanceImage->GetGeometry()->GetOrigin());

  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ReduceFilter->GetNumberOfOutputs(); i++)
  {
    polyDataAppender->AddInput(m_ReduceFilter->GetOutput(i)->GetVtkPolyData());
  }
  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  m_InterpolationResult->DisconnectPipeline();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult()
{
    return m_InterpolationResult;
}

mitk::Surface* mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}

void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage &ds)
{
  m_DataStorage = &ds;
}

unsigned int mitk::SurfaceInterpolationController::CreateNewContourList()
{
  unsigned int newID =  m_ListOfContourLists.size();
  ContourPositionPairList newList;
  m_ListOfContourLists.push_back(newList);
  this->SetCurrentListID(newID);
  m_InterpolationResult = 0;
  return m_CurrentContourListID;
}

void mitk::SurfaceInterpolationController::SetCurrentListID ( unsigned int ID )
{
  if (ID == m_CurrentContourListID )
    return;

  m_CurrentContourListID = ID;

  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();
  
  for (unsigned int i = 0; i < m_ListOfContourLists.at(m_CurrentContourListID).size(); i++)
  {
    m_ReduceFilter->SetInput(i, m_ListOfContourLists.at(m_CurrentContourListID).at(i).contour);
    m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }
}

void mitk::SurfaceInterpolationController::SetMinSpacing(double minSpacing)
{
  m_ReduceFilter->SetMinSpacing(minSpacing);
}

void mitk::SurfaceInterpolationController::SetMaxSpacing(double maxSpacing)
{
  m_ReduceFilter->SetMaxSpacing(maxSpacing);
  m_NormalsFilter->SetMaxSpacing(maxSpacing);
}

void mitk::SurfaceInterpolationController::SetDistanceImageVolume(unsigned int distImgVolume)
{
  m_InterpolateSurfaceFilter->SetDistanceImageVolume(distImgVolume);
}

void mitk::SurfaceInterpolationController::SetWorkingImage(Image* workingImage)
{
  m_NormalsFilter->SetSegmentationBinaryImage(workingImage);
}

mitk::Image* mitk::SurfaceInterpolationController::GetImage()
{
  return m_InterpolateSurfaceFilter->GetOutput();
}
